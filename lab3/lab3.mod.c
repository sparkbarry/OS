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
	{ 0x058c185a, "jiffies" },
	{ 0x12ca6142, "ktime_get_with_offset" },
	{ 0xe8213e80, "_printk" },
	{ 0xc7ea9460, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0x058c185a,
	0x12ca6142,
	0xe8213e80,
	0xc7ea9460,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"jiffies\0"
	"ktime_get_with_offset\0"
	"_printk\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "61CDEE366F15E66B90DB8A2");
