#include<linux/module.h>
#include<linux/platform_device.h>
#include"platfrom.h"

//creatre 2 platfrom device
struct pcdevice_platfrom_data pcd_pdata[2]={
	[0]={.size=512, .per=777, .serial_number="ABCD1111"},
	[1]={.size=512, .per=777, .serial_number="ABCD2222"}
};

struct platform_device pltfrom_device_1={
	.name="pcd_char_dev",
	.id=0,
	.dev={.platform_data=&pcd_pdata[1]}
};
struct platform_device pltfrom_device_2={.name="pcd_char_dev",
	.id=1,
	.dev={.platform_data=&pcd_pdata[1]}
};
void pcdev_release(struct device *dev)
{
printk("relrase fuction call\n");
}
static int __init pltfrom_init(void)
{
	// register device
	platform_device_register(&pltfrom_device_1);
	platform_device_register(&pltfrom_device_2);
printk("init fuction call\n");
	return 0;
}
void __exit pltfrom_exit(void)
{
	platform_device_unregister(&pltfrom_device_1);
	platform_device_unregister(&pltfrom_device_2);
printk("exit fuction call\n");
}

module_init(pltfrom_init);
module_exit(pltfrom_exit);

MODULE_DESCRIPTION("DIGAMBAR CDEV INIT ADD");
MODULE_LICENSE("GPL");
