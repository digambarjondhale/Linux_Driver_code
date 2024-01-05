
// header diclrtaion 
//#include<linux/kernel.h>
//#include<linux/init.h>
#include<linux/module.h>
 
/*
** Module Init function
*/
static int __init hello_init(void)
{
 	pr_info("test info message\n");
	printk(KERN_INFO "Diagmabr Kernel Module Inserted\n");
    return 0;
}

/*
** Module Exit function
*/
static void __exit hello_exit(void)
{
    printk(KERN_INFO "Digambar Kernel Module Removed\n");
}
 // module register
module_init(hello_init);
module_exit(hello_exit);
 // module discreption 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Digambar Jondhale");
MODULE_DESCRIPTION("A simple hello world driver");
