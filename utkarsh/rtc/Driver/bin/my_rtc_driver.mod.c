#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
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

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif


static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
	{ 0x92997ed8, "_printk" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0x80ca5026, "_bin2bcd" },
	{ 0x25a360b8, "i2c_transfer" },
	{ 0xb623b595, "_dev_err" },
	{ 0x3ea1b6e4, "__stack_chk_fail" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0xb6936ffe, "_bcd2bin" },
	{ 0xae353d77, "arm_copy_from_user" },
	{ 0x5f754e5a, "memset" },
	{ 0x2cfde9a2, "warn_slowpath_fmt" },
	{ 0x328a05f1, "strncpy" },
	{ 0xbcab6ee6, "sscanf" },
	{ 0x7482d442, "i2c_smbus_write_byte_data" },
	{ 0xbd908632, "i2c_unregister_device" },
	{ 0xec85562e, "i2c_del_driver" },
	{ 0xe6ab7b50, "device_destroy" },
	{ 0x3a8831fe, "class_destroy" },
	{ 0x2bf9031b, "cdev_del" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x61907396, "__class_create" },
	{ 0x5fe2f79b, "device_create" },
	{ 0x729bab70, "cdev_init" },
	{ 0x95c1026a, "cdev_add" },
	{ 0xe702d4c8, "kmalloc_caches" },
	{ 0x5995f766, "kmalloc_trace" },
	{ 0xa29492ba, "i2c_get_adapter" },
	{ 0xb38eba6e, "i2c_new_client_device" },
	{ 0x36948ecd, "i2c_register_driver" },
	{ 0xdddede76, "i2c_put_adapter" },
	{ 0xfbc38a77, "i2c_smbus_read_byte_data" },
	{ 0xfc6de7ff, "devm_rtc_device_register" },
	{ 0xc358aaf8, "snprintf" },
	{ 0x51a910c0, "arm_copy_to_user" },
	{ 0xc84d16dc, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "B8BF755452FC956E5C14B2F");
