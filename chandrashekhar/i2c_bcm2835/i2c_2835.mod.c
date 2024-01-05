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
	{ 0xe707d823, "__aeabi_uidiv" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0x92997ed8, "_printk" },
	{ 0x3507959a, "__platform_driver_register" },
	{ 0xea3c74e, "tasklet_kill" },
	{ 0x37a0cba, "kfree" },
	{ 0xacb4d88c, "clk_rate_exclusive_put" },
	{ 0xb6e6d99d, "clk_disable" },
	{ 0xb077e70a, "clk_unprepare" },
	{ 0xc1514a3b, "free_irq" },
	{ 0x8196066c, "i2c_del_adapter" },
	{ 0xc37335b0, "complete" },
	{ 0xca54fee, "_test_and_set_bit" },
	{ 0x9d2ab8ac, "__tasklet_schedule" },
	{ 0xb623b595, "_dev_err" },
	{ 0xc4bda6b9, "devm_kmalloc" },
	{ 0x870d5a1c, "__init_swait_queue_head" },
	{ 0x4c711650, "platform_get_resource" },
	{ 0x428fc6de, "devm_ioremap_resource" },
	{ 0xa8cc5fa2, "devm_clk_get" },
	{ 0xc358aaf8, "snprintf" },
	{ 0xc569d8ce, "__clk_get_name" },
	{ 0x4871d75d, "clk_hw_register_clkdev" },
	{ 0x46cdddaf, "devm_clk_register" },
	{ 0x5aae23d4, "of_property_read_variable_u32_array" },
	{ 0xc5604800, "clk_set_rate_exclusive" },
	{ 0x7c9a7371, "clk_prepare" },
	{ 0x815588a6, "clk_enable" },
	{ 0x161b7a01, "platform_get_irq" },
	{ 0x92d5838e, "request_threaded_irq" },
	{ 0xe439b94c, "of_device_get_match_data" },
	{ 0x6e68c78a, "i2c_add_adapter" },
	{ 0xe702d4c8, "kmalloc_caches" },
	{ 0x5995f766, "kmalloc_trace" },
	{ 0x2364c85a, "tasklet_init" },
	{ 0x23f61eea, "dev_err_probe" },
	{ 0x3ea1b6e4, "__stack_chk_fail" },
	{ 0x18bcca39, "_dev_warn" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0x60ad2629, "platform_driver_unregister" },
	{ 0xa16b21fb, "wait_for_completion_timeout" },
	{ 0x248409e9, "param_ops_uint" },
	{ 0xc84d16dc, "module_layout" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("of:N*T*Cbrcm,bcm2711-i2c");
MODULE_ALIAS("of:N*T*Cbrcm,bcm2711-i2cC*");
MODULE_ALIAS("of:N*T*Cbrcm,bcm2835-i2c");
MODULE_ALIAS("of:N*T*Cbrcm,bcm2835-i2cC*");

MODULE_INFO(srcversion, "B9EA1752AA0C5239739D4C5");
