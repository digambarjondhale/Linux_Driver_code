#include<linux/module.h>
#include<linux/fs.h>

#define DEV_MES_SIZE 512

/* pesudo code device memory */
char device_buffer[DEV_MES_SIZE];

/* hold device number */
dev_t device_number;

/* init */
static int __init pcd_driver_init(void)
{

	printk("int pcd call \n");
	/* dynamically allocated deivce number */
	alloc_chrdev_region(&device_number,0,7,"digambar_char_pcd");

	printk("device number=%u \n",device_number);
	return 0;
}

/* exit */
static void __exit pcd_driver_exit(void)
{
	printk("exit for pcd_drivrs\n");
}

/* register the init and exit */

module_init(pcd_driver_init);
module_exit(pcd_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Digambar softnautics");
MODULE_DESCRIPTION("pesudo code char");

