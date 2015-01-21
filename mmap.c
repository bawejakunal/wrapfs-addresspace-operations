/*
 * Copyright (c) 1998-2011 Erez Zadok
 * Copyright (c) 2009	   Shrikar Archak
 * Copyright (c) 2003-2011 Stony Brook University
 * Copyright (c) 2003-2011 The Research Foundation of SUNY
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "wrapfs.h"
#include <linux/page-flags.h>
#include <linux/highmem.h>
#include <linux/pagemap.h>
#include <linux/writeback.h>
#include <linux/crypto.h>


/**
 * wrapfs_read_lower
 * @data: The read data is stored here by this function
 * @offset: Byte offset in the lower file from which to read the data
 * @size: Number of bytes to read from @offset of the lower file and
 *        store into @data
 * @wrapfs_inode: The wrapfs inode
 *
 * Read @size bytes of data at byte offset @offset from the lower
 * inode into memory location @data.
 *
 * Returns bytes read on success; 0 on EOF; less than zero on error
 */
int wrapfs_read_lower(char *data, loff_t offset, size_t size,
                        struct inode *wrapfs_inode, struct file *file)
{
	struct file *lower_file;
    mm_segment_t fs_save;
    ssize_t rc;
	mode_t previous_mode;
      
	lower_file = wrapfs_lower_file(file);
    if (!lower_file)
            return -EIO;
    fs_save = get_fs();
    set_fs(get_ds());
	previous_mode = lower_file->f_mode;
	lower_file->f_mode |= FMODE_READ;
    rc = vfs_read(lower_file, data, size, &offset);
	lower_file->f_mode = previous_mode;
    set_fs(fs_save);
    return rc;
}

int wrapfs_write_lower(struct inode *wrapfs_inode, char *data,
                         loff_t offset, size_t size, struct file *file)
{
        struct file *lower_file;
        mm_segment_t fs_save;
        ssize_t rc;

        lower_file = wrapfs_lower_file(file);
        if (!lower_file)
                return -EIO;
        fs_save = get_fs();
        set_fs(get_ds());

        rc = vfs_write(lower_file, data, size, &offset);
	
        set_fs(fs_save);
        mark_inode_dirty_sync(wrapfs_inode);
        return rc;
}

/**
 * wrapfs_read_lower_page_segment
 * @page_for_wrapfs: The page into which data for wrapfs will be
 *                     written
 * @offset_in_page: Offset in @page_for_wrapfs from which to start
 *                  writing
 * @size: The number of bytes to write into @page_for_wrapfs
 * @wrapfs_inode: The wrapfs inode
 *
 * Determines the byte offset in the file for the given page and
 * offset within the page, maps the page, and makes the call to read
 * the contents of @page_for_wrapfs from the lower inode.
 *
 * Returns zero on success; non-zero otherwise
 */
int wrapfs_read_lower_page_segment(struct page *page_for_wrapfs,
                                     pgoff_t page_index,
                                     size_t offset_in_page, size_t size,
                                     struct inode *wrapfs_inode,
				     		 struct file *file)
{
        char *virt;
        loff_t offset;
        int rc;

        offset = ((((loff_t)page_index) << PAGE_CACHE_SHIFT) + offset_in_page);
        virt = kmap(page_for_wrapfs);
        rc = wrapfs_read_lower(virt, offset, size, wrapfs_inode, file);
        if (rc > 0)
                rc = 0;
        kunmap(page_for_wrapfs);
        flush_dcache_page(page_for_wrapfs);
        return rc;
}

/**
 * wrapfs_write_lower_page_segment
 * @wrapfs_inode: The wrapfs inode
 * @page_for_lower: The page containing the data to be written to the
 *                  lower file
 * @offset_in_page: The offset in the @page_for_lower from which to
 *                  start writing the data
 * @size: The amount of data from @page_for_lower to write to the
 *        lower file
 *
 * Determines the byte offset in the file for the given page and
 * offset within the page, maps the page, and makes the call to write
 * the contents of @page_for_lower to the lower inode.
 *
 * Returns zero on success; non-zero otherwise
 */
int wrapfs_write_lower_page_segment(struct inode *wrapfs_inode,
                                      struct page *page_for_lower,
                                      size_t offset_in_page, size_t size, 
                                      struct file *file)
{
        char *virt;
        loff_t offset;
        int rc = -1;

        offset = ((((loff_t)page_for_lower->index) << PAGE_CACHE_SHIFT)
                  + offset_in_page);
        virt = kmap(page_for_lower);
		rc = wrapfs_write_lower(wrapfs_inode, virt, offset, size, file);
        if (rc > 0)
                rc = 0;
        kunmap(page_for_lower);
        return rc;
}

static int wrapfs_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
	int err;
	struct file *file, *lower_file;
	const struct vm_operations_struct *lower_vm_ops;
	struct vm_area_struct lower_vma;

	printk(KERN_INFO "In wrapfs_fault()");

	memcpy(&lower_vma, vma, sizeof(struct vm_area_struct));
	file = lower_vma.vm_file;
	lower_vm_ops = WRAPFS_F(file)->lower_vm_ops;
	BUG_ON(!lower_vm_ops);

	lower_file = wrapfs_lower_file(file);
	/*
	 * XXX: vm_ops->fault may be called in parallel.  Because we have to
	 * resort to temporarily changing the vma->vm_file to point to the
	 * lower file, a concurrent invocation of wrapfs_fault could see a
	 * different value.  In this workaround, we keep a different copy of
	 * the vma structure in our stack, so we never expose a different
	 * value of the vma->vm_file called to us, even temporarily.  A
	 * better fix would be to change the calling semantics of ->fault to
	 * take an explicit file pointer.
	 */
	lower_vma.vm_file = lower_file;
	err = lower_vm_ops->fault(&lower_vma, vmf);
	return err;
}

static int wrapfs_page_mkwrite(struct vm_area_struct *vma,
                   struct vm_fault *vmf)
{
    int err = 0;
    struct file *file, *lower_file;
    const struct vm_operations_struct *lower_vm_ops;
    struct vm_area_struct lower_vma;

    memcpy(&lower_vma, vma, sizeof(struct vm_area_struct));
    file = lower_vma.vm_file;
    lower_vm_ops = WRAPFS_F(file)->lower_vm_ops;
    BUG_ON(!lower_vm_ops);
    if (!lower_vm_ops->page_mkwrite)
        goto out;

    lower_file = wrapfs_lower_file(file);
    /*
     * XXX: vm_ops->page_mkwrite may be called in parallel.
     * Because we have to resort to temporarily changing the
     * vma->vm_file to point to the lower file, a concurrent
     * invocation of wrapfs_page_mkwrite could see a different
     * value.  In this workaround, we keep a different copy of the
     * vma structure in our stack, so we never expose a different
     * value of the vma->vm_file called to us, even temporarily.
     * A better fix would be to change the calling semantics of
     * ->page_mkwrite to take an explicit file pointer.
     */
    lower_vma.vm_file = lower_file;
    err = lower_vm_ops->page_mkwrite(&lower_vma, vmf);
out:
    return err;
}

/**
 * wrapfs_writepage
 * @page: Page that is locked before this call is made
 *
 * Returns zero on success; non-zero otherwise
 */
static int wrapfs_writepage(struct page *page, struct writeback_control *wbc)
{
    int rc =0;
    printk(KERN_INFO "in wrapfs_writepage");
    SetPageUptodate(page);
    unlock_page(page);
    return rc;
}

static int wrapfs_readpage(struct file *file, struct page *page)
{
	int err=0;

    printk(KERN_INFO "in wrapfs_readpage");

	err = wrapfs_read_lower_page_segment(page, page->index, 0, PAGE_CACHE_SIZE, page->mapping->host, file);
	if (err) {
		printk(KERN_ERR "Error reading page; err = "
				 "[%d]\n", err);
		goto out;
	}

out:
	if (err == 0)
		SetPageUptodate(page);
	else
		ClearPageUptodate(page);
	unlock_page(page);
	return err;
}

static int wrapfs_write_begin(struct file *file,
                         struct address_space *mapping,
                         loff_t pos, unsigned len, unsigned flags,
                         struct page **pagep, void **fsdata)
{
        pgoff_t index = pos >> PAGE_CACHE_SHIFT;
        struct page *page;
        loff_t prev_page_end_size;
        int rc = 0;

        page = grab_cache_page_write_begin(mapping, index, flags);
        if (!page)
                return -ENOMEM;
        *pagep = page;

        prev_page_end_size = ((loff_t)index << PAGE_CACHE_SHIFT);
        if (!PageUptodate(page)) {
		rc = wrapfs_read_lower_page_segment(
		page, index, 0, PAGE_CACHE_SIZE,
		mapping->host, file);
		if (rc) {
			printk(KERN_ERR "%s: Error reading "
				   "page; rc = [%d]\n",
				   __func__, rc);
			ClearPageUptodate(page);
			goto out;
		}
		SetPageUptodate(page);
        }

        /* Writing to a new page, and creating a small hole from start
         * of page?  Zero it out. */
        if ((i_size_read(mapping->host) == prev_page_end_size)
            && (pos != 0))
                zero_user(page, 0, PAGE_CACHE_SIZE);
out:
        if (unlikely(rc)) {
                unlock_page(page);
                page_cache_release(page);
                *pagep = NULL;
        }
        return rc;
}


/**
 * @file: The wrapfs file object
 * @mapping: The wrapfs object
 * @pos: The file position
 * @len: The length of the data (unused)
 * @copied: The amount of data copied
 * @page: The wrapfs page
 * @fsdata: The fsdata (unused)
 *
 * This is where we encrypt the data and pass the encrypted data to
 * the lower filesystem.
 */
static int wrapfs_write_end(struct file *file,
                         struct address_space *mapping,
                         loff_t pos, unsigned len, unsigned copied,
                         struct page *page, void *fsdata)
{
    //pgoff_t index = pos >> PAGE_CACHE_SHIFT;
    unsigned from = pos & (PAGE_CACHE_SIZE - 1);
    unsigned to = from + copied;
    struct inode *wrapfs_inode = mapping->host;
    int rc;
    //int need_unlock_page = 1;

    rc = wrapfs_write_lower_page_segment(wrapfs_inode, page, 0,
                                           to, file);
    if (!rc) {
            rc = copied;
            fsstack_copy_inode_size(wrapfs_inode,
                    wrapfs_lower_inode(wrapfs_inode));
    }
//out:
    //if (need_unlock_page)
    unlock_page(page);
    page_cache_release(page);
    return rc;
}

static sector_t wrapfs_bmap(struct address_space *mapping, sector_t block)
{
    int rc = 0;
    struct inode *inode;
    struct inode *lower_inode;

    inode = (struct inode *)mapping->host;
    lower_inode = wrapfs_lower_inode(inode);
    if (lower_inode->i_mapping->a_ops->bmap)
        rc = lower_inode->i_mapping->a_ops->bmap(lower_inode->i_mapping,
                             block);
    return rc;
}

static ssize_t wrapfs_direct_IO(int rw, struct kiocb *iocb,
                const struct iovec *iov, loff_t offset,
                unsigned long nr_segs)
{
    /*
     * This function should never be called directly.  We need it
     * to exist, to get past a check in open_check_o_direct(),
     * which is called from do_last().
     */
    return -EINVAL;
}

/*
 * XXX: the default address_space_ops for wrapfs is empty.  We cannot set
 * our inode->i_mapping->a_ops to NULL because too many code paths expect
 * the a_ops vector to be non-NULL.
 */
const struct address_space_operations wrapfs_dummy_aops = {
	/* empty on purpose */
};

const struct address_space_operations wrapfs_aops = {
	/* empty on purpose */
        .direct_IO = wrapfs_direct_IO,
        .readpage = wrapfs_readpage,
        .writepage = wrapfs_writepage,
        .write_begin = wrapfs_write_begin,
        .write_end = wrapfs_write_end,
        .bmap = wrapfs_bmap,
};

const struct vm_operations_struct wrapfs_vm_ops = {
	.fault		= wrapfs_fault,
    .page_mkwrite = wrapfs_page_mkwrite,
};