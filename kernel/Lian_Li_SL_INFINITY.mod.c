#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x02f9bbf0, "init_timer_key" },
	{ 0x058c185a, "jiffies" },
	{ 0x32feeafc, "mod_timer" },
	{ 0xb1a58910, "hid_unregister_driver" },
	{ 0xbd03ed67, "random_kmalloc_seed" },
	{ 0xc2fefbb5, "kmalloc_caches" },
	{ 0x38395bf3, "__kmalloc_cache_noprof" },
	{ 0x888b8f57, "strcmp" },
	{ 0x092a35a2, "_copy_to_user" },
	{ 0xcb8b6ec6, "kfree" },
	{ 0xaef1f20d, "system_wq" },
	{ 0x49733ad6, "queue_work_on" },
	{ 0x1b3db703, "param_ops_int" },
	{ 0xd272d446, "__fentry__" },
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0xe8213e80, "_printk" },
	{ 0xd7488097, "__hid_register_driver" },
	{ 0x4c5fe0a9, "hid_hw_stop" },
	{ 0x2352b148, "timer_delete_sync" },
	{ 0x2d88a3ab, "cancel_work_sync" },
	{ 0xb9e81daf, "proc_remove" },
	{ 0x90a48d82, "__ubsan_handle_out_of_bounds" },
	{ 0xa61fd7aa, "__check_object_size" },
	{ 0x092a35a2, "_copy_from_user" },
	{ 0x173ec8da, "sscanf" },
	{ 0xd272d446, "__stack_chk_fail" },
	{ 0xdd6830c7, "sprintf" },
	{ 0x437e81c7, "simple_read_from_buffer" },
	{ 0x76bae7c3, "hid_hw_raw_request" },
	{ 0xae70f3e0, "hid_open_report" },
	{ 0xe605c857, "hid_hw_start" },
	{ 0xda212183, "proc_mkdir" },
	{ 0xf8d7ac5e, "proc_create" },
	{ 0xf4d25b68, "proc_create_data" },
	{ 0x70eca2ca, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0x02f9bbf0,
	0x058c185a,
	0x32feeafc,
	0xb1a58910,
	0xbd03ed67,
	0xc2fefbb5,
	0x38395bf3,
	0x888b8f57,
	0x092a35a2,
	0xcb8b6ec6,
	0xaef1f20d,
	0x49733ad6,
	0x1b3db703,
	0xd272d446,
	0xd272d446,
	0xe8213e80,
	0xd7488097,
	0x4c5fe0a9,
	0x2352b148,
	0x2d88a3ab,
	0xb9e81daf,
	0x90a48d82,
	0xa61fd7aa,
	0x092a35a2,
	0x173ec8da,
	0xd272d446,
	0xdd6830c7,
	0x437e81c7,
	0x76bae7c3,
	0xae70f3e0,
	0xe605c857,
	0xda212183,
	0xf8d7ac5e,
	0xf4d25b68,
	0x70eca2ca,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"init_timer_key\0"
	"jiffies\0"
	"mod_timer\0"
	"hid_unregister_driver\0"
	"random_kmalloc_seed\0"
	"kmalloc_caches\0"
	"__kmalloc_cache_noprof\0"
	"strcmp\0"
	"_copy_to_user\0"
	"kfree\0"
	"system_wq\0"
	"queue_work_on\0"
	"param_ops_int\0"
	"__fentry__\0"
	"__x86_return_thunk\0"
	"_printk\0"
	"__hid_register_driver\0"
	"hid_hw_stop\0"
	"timer_delete_sync\0"
	"cancel_work_sync\0"
	"proc_remove\0"
	"__ubsan_handle_out_of_bounds\0"
	"__check_object_size\0"
	"_copy_from_user\0"
	"sscanf\0"
	"__stack_chk_fail\0"
	"sprintf\0"
	"simple_read_from_buffer\0"
	"hid_hw_raw_request\0"
	"hid_open_report\0"
	"hid_hw_start\0"
	"proc_mkdir\0"
	"proc_create\0"
	"proc_create_data\0"
	"module_layout\0"
;

MODULE_INFO(depends, "hid");

MODULE_ALIAS("hid:b0003g*v00000CF2p0000A102");

MODULE_INFO(srcversion, "C8A60F448E6C66B80521992");
