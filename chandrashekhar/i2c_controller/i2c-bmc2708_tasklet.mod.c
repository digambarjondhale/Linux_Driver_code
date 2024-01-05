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
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
	{ 0x3507959a, "__platform_driver_register" },
	{ 0xea3c74e, "tasklet_kill" },
	{ 0x37a0cba, "kfree" },
	{ 0x8196066c, "i2c_del_adapter" },
	{ 0xc1514a3b, "free_irq" },
	{ 0xedc03953, "iounmap" },
	{ 0xb6e6d99d, "clk_disable" },
	{ 0xb077e70a, "clk_unprepare" },
	{ 0x2e1ca751, "clk_put" },
	{ 0x92997ed8, "_printk" },
	{ 0xca54fee, "_test_and_set_bit" },
	{ 0x9d2ab8ac, "__tasklet_schedule" },
	{ 0x60ad2629, "platform_driver_unregister" },
	{ 0xde55e795, "_raw_spin_lock_irqsave" },
	{ 0xf3d0b495, "_raw_spin_unlock_irqrestore" },
	{ 0xa16b21fb, "wait_for_completion_timeout" },
	{ 0xb623b595, "_dev_err" },
	{ 0xae577d60, "_raw_spin_lock" },
	{ 0xc37335b0, "complete" },
	{ 0xe707d823, "__aeabi_uidiv" },
	{ 0xa30c2682, "of_alias_get_id" },
	{ 0x5aae23d4, "of_property_read_variable_u32_array" },
	{ 0x4c711650, "platform_get_resource" },
	{ 0x161b7a01, "platform_get_irq" },
	{ 0x580b28e6, "clk_get" },
	{ 0x7c9a7371, "clk_prepare" },
	{ 0x815588a6, "clk_enable" },
	{ 0xe702d4c8, "kmalloc_caches" },
	{ 0x5995f766, "kmalloc_trace" },
	{ 0x73e20c1c, "strlcpy" },
	{ 0x870d5a1c, "__init_swait_queue_head" },
	{ 0xe97c4103, "ioremap" },
	{ 0x92d5838e, "request_threaded_irq" },
	{ 0x2364c85a, "tasklet_init" },
	{ 0x30c3d2d7, "i2c_add_numbered_adapter" },
	{ 0x556e4390, "clk_get_rate" },
	{ 0x263b1492, "_dev_info" },
	{ 0x3ea1b6e4, "__stack_chk_fail" },
	{ 0x18bcca39, "_dev_warn" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0x50f6efc9, "param_ops_bool" },
	{ 0x248409e9, "param_ops_uint" },
	{ 0xc84d16dc, "module_layout" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("of:N*T*Cbrcm,bcm2708-i2c");
MODULE_ALIAS("of:N*T*Cbrcm,bcm2708-i2cC*");

MODULE_INFO(srcversion, "C0DDFF6EC5C4DD6C5A66D40");
