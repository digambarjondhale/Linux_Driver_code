#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/err.h>
#include<linux/kdev_t.h>
#include<linux/slab.h>
#include<linux/cdev.h>
#include<linux/device.h>

#define MAX_SIZE 1024

dev_t dev=0;
static struct class *dev_class;
static struct cdev mycdev;
uint8_t *kbuff;

static int __init hello_entry(void);
static void __exit hello_exit(void);
static int my_open(struct inode *inode, struct file *file);
static int my_release(struct inode *inode, struct file *file);
static ssize_t my_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t my_write(struct file *filp, const char *buf, size_t len, loff_t * off);

static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = my_read,
        .write          = my_write,
        .open           = my_open,
        .release        = my_release,
};


static int my_open(struct inode *inode, struct file *file)
{
	pr_info("device file opened......!!!\n");
	return 0;

}
static int my_release(struct inode *inode, struct file *file)
{
	pr_info("device file closed......!!!\n");
	return 0;
}	
static ssize_t  my_read(struct file *filp, char __user *buf, size_t len,loff_t * off)
{
	if(copy_to_user(buf,kbuff,MAX_SIZE))
	{
		pr_err("data error:ERROR!\n");

	}
	pr_info("data_read:DONE!\n");
	return MAX_SIZE;

}	
static ssize_t  my_write(struct file *filp, const char *buf, size_t len, loff_t * off)
{
	if(copy_from_user(kbuff,buf,len))
	{
		pr_err("data error:ERROR!\n");
	}
	pr_info("data_write:DONE!\n");
	return len;


}
static int __init hello_entry(void)
{
	printk(KERN_INFO "entry function of basic char_dev code\n");
	
	if(alloc_chrdev_region(&dev,0,1,"mydev")<0)
	{
		pr_info("Cannot allocate major number\n");
                return -1;
        }
        pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));

	cdev_init(&mycdev,&fops);

	if((cdev_add(&mycdev,dev,1)) < 0){
            pr_err("Cannot add the device to the system\n");
            goto r_class;
        }
	
	if(IS_ERR(dev_class =class_create(THIS_MODULE, "my_class")))
	{
		pr_info("Cannot create the struct class\n");
        	goto r_class;
	}
	if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"my_device")))
	{
		pr_info("Cannot create the device\n");
        	goto r_device;
	}
	if((kbuff = kmalloc(MAX_SIZE , GFP_KERNEL)) == 0)
	{
        	pr_info("Cannot allocate memory in kernel\n");
        	goto r_device;
        }
        
        strcpy(kbuff, "Hello_Softnautics");
        
        pr_info("Device Driver Insert...Done!!!\n");

	
	
	return 0;

r_device:
	class_destroy(dev_class);
r_class:
	unregister_chrdev_region(dev,1);
	return -1;	
}
static void __exit hello_exit(void)
{
	kfree(kbuff);
	device_destroy(dev_class,dev);
	class_destroy(dev_class);
	cdev_del(&mycdev);
	unregister_chrdev_region(dev,1);
	printk(KERN_INFO "exit function of basic char_dev code\n");


}	
module_init (hello_entry);
module_exit (hello_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("chandrashekhar");
MODULE_DESCRIPTION("A simple charecter driver");


