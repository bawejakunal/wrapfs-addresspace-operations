#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xaf6e7a5f, "module_layout" },
	{ 0x841fde82, "vfs_create" },
	{ 0xca880822, "kmem_cache_destroy" },
	{ 0x5baa1766, "kmalloc_caches" },
	{ 0x2f588f66, "fsstack_copy_inode_size" },
	{ 0x4c4fef19, "kernel_stack" },
	{ 0xe78dcaa6, "generic_file_llseek" },
	{ 0x3d2b08ff, "__mark_inode_dirty" },
	{ 0xf58d3676, "mntget" },
	{ 0x6d178c44, "vfs_readdir" },
	{ 0x89bd8398, "inode_permission" },
	{ 0x25ec1b28, "strlen" },
	{ 0xa331f201, "d_set_d_op" },
	{ 0xe91f3774, "iget5_locked" },
	{ 0xb32ee15e, "dget_parent" },
	{ 0x15c11a50, "vfs_link" },
	{ 0xf4e94a85, "generic_delete_inode" },
	{ 0xd238d502, "generic_file_aio_read" },
	{ 0x56940c26, "dput" },
	{ 0x44e9a829, "match_token" },
	{ 0xc4781c06, "dentry_open" },
	{ 0x4eb75782, "vfs_path_lookup" },
	{ 0x368a64db, "vfs_mknod" },
	{ 0x6d2342e3, "end_writeback" },
	{ 0x6a22c930, "mutex_unlock" },
	{ 0x85df9b6c, "strsep" },
	{ 0x36796a77, "generic_read_dir" },
	{ 0x4c8b50a8, "igrab" },
	{ 0x7c0cd982, "vfs_symlink" },
	{ 0xa86e501c, "mount_nodev" },
	{ 0xd9c4bbf5, "path_get" },
	{ 0x6c8b8b69, "generic_file_aio_write" },
	{ 0x81e0e27b, "mnt_drop_write" },
	{ 0xe1b5e618, "set_page_dirty" },
	{ 0xf45b7a1c, "truncate_setsize" },
	{ 0x9930b77d, "vfs_rmdir" },
	{ 0x9d1f5644, "unlock_rename" },
	{ 0xfd0f8531, "vfs_read" },
	{ 0xce31fdc7, "kern_path" },
	{ 0x7e99aa8f, "current_task" },
	{ 0x27e1a049, "printk" },
	{ 0x6018533e, "d_rehash" },
	{ 0x343a9614, "find_or_create_page" },
	{ 0x54cf92df, "d_alloc_root" },
	{ 0xa1c76e0a, "_cond_resched" },
	{ 0xb4390f9a, "mcount" },
	{ 0xe7311dd5, "kmem_cache_free" },
	{ 0xbd6316e4, "lock_rename" },
	{ 0x6d03189c, "mutex_lock" },
	{ 0x4e3567f7, "match_int" },
	{ 0xfecb9775, "wait_on_page_bit" },
	{ 0x6a0c717f, "unlock_page" },
	{ 0x93904c60, "fput" },
	{ 0x2f3eb5d6, "inode_init_once" },
	{ 0xce92f63f, "kmem_cache_alloc" },
	{ 0x33b84f74, "copy_page" },
	{ 0x733c3e77, "mnt_want_write" },
	{ 0x6ef6d122, "d_alloc" },
	{ 0xd08d3492, "do_munmap" },
	{ 0xa66e7e46, "do_sync_read" },
	{ 0x682821df, "unlock_new_inode" },
	{ 0x4d52ffcb, "d_drop" },
	{ 0xc6df0c72, "inode_newsize_ok" },
	{ 0x4fbaa59a, "vfs_statfs" },
	{ 0x1e2fd5fc, "vfs_mkdir" },
	{ 0x94c84a0c, "clear_page_dirty_for_io" },
	{ 0x5d9006ee, "inode_change_ok" },
	{ 0xd9325542, "path_put" },
	{ 0xaa44359f, "kmem_cache_alloc_trace" },
	{ 0xd52bf1ce, "_raw_spin_lock" },
	{ 0x44c542d8, "generic_show_options" },
	{ 0x92aff4bf, "vfs_unlink" },
	{ 0xe30ebac3, "kmem_cache_create" },
	{ 0xf2f662d, "register_filesystem" },
	{ 0x13c1ff64, "fsstack_copy_attr_all" },
	{ 0x3327836c, "d_lookup" },
	{ 0xe9cdd4b0, "iput" },
	{ 0x4006d5e2, "generic_file_fsync" },
	{ 0x37a0cba, "kfree" },
	{ 0x6bd807c, "do_sync_write" },
	{ 0xb724b1a1, "put_page" },
	{ 0xa0194db, "unregister_filesystem" },
	{ 0x2254f30c, "init_special_inode" },
	{ 0x3e8a1da, "vfs_rename" },
	{ 0xd1757f71, "grab_cache_page_write_begin" },
	{ 0x23155c0c, "notify_change" },
	{ 0xb01f4416, "d_instantiate" },
	{ 0xe879515d, "vfs_write" },
	{ 0xa49e4e01, "vfs_fsync_range" },
	{ 0x5d4579ed, "truncate_inode_pages" },
	{ 0xec467a6f, "generic_shutdown_super" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "3BD9941E7BD95C1E5AF979C");
