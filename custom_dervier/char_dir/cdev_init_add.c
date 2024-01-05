#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#define SIZE_BUF 512

char device_buffer[SIZE_BUF];
struct cdev pcd_dev;
struct file_operations pcd_fops;
dev_t device_number;
static int __init cdev_int_add(void)
{
	/* dynamically allocated a device number */
	alloc_chrdev_region(&device_number,0,1,"pcd");

	/* initlize cdev struct with fops*/
	cdev_init(&pcd_dev,&pcd_fops);

	/* register cdev  structure with VFS*/
	pcd_dev.owner=THIS_MODULE;
	cdev_add(&pcd_dev,device_number,7);

	printk("init call for cdev int add\n");
	return 0;
}
static void __exit cdev_int_add_exit(void)
{
	printk("exit for cdev init add\n");
}

module_init(cdev_int_add);
module_exit(cdev_int_add_exit);

MODULE_DESCRIPTION("DAGMABR CDEV INIT ADD");
MODULE_LICENSE("GPL");

