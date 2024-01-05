#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/kdev_t.h>
#include<linux/uaccess.h>
#define DEV_MEM_SIZE 512

char device_buffer[DEV_MEM_SIZE];

struct cdev pcd_dev;
dev_t device_number;
struct class *class_pcd;
struct device *device_pcd;

loff_t pcd_llseek (struct file *flip, loff_t offset , int where)
{
	switch(where)
	{
		case SEEK_SET:
			flip->f_pos=offset;
			break;
		case SEEK_CUR:
			flip->f_pos+=offset;
			break;
		case SEEK_END:
			flip->f_pos=DEV_MEM_SIZE + offset;
			break;
		default:
			printk("error in pcd lseek \n");
			return EINVAL;
			printk("pcd_llseek\n");
	}
	return flip->f_pos;
}
ssize_t pcd_read(struct file * filp,char __user *buff,size_t count ,loff_t *f_pos)
{
	printk("befor read opration offset count %lld\n",*f_pos);
	/* adjust count */
	if((*f_pos+count)>DEV_MEM_SIZE)                                         
		count=DEV_MEM_SIZE - *f_pos;

	/*copy to user */
	if(copy_to_user(buff,&device_buffer[*f_pos],count))
		printk("Error occuer in copy to user\n");


	/* update the offset */
	*f_pos +=count;
	printk("after read oprtion offset count %lld\n",*f_pos);

	return count;
}
ssize_t pcd_write (struct file *filp,const char __user *buff,size_t count, loff_t *f_pos)
{

	printk("befor write data offset %lld\n",*f_pos);
	/* adjust count */
	if((*f_pos+count) >DEV_MEM_SIZE)
		count=DEV_MEM_SIZE - *f_pos;

	/* copy from user */
	if(copy_from_user(&device_buffer[*f_pos],buff,count))
		printk("error for writing the data \n");

	/* update offset */
	*f_pos += count;
	printk("pcd_write\n");
	printk("after write data offset %lld\n",*f_pos);

	return count;
}
int pcd_release(struct inode *inode, struct file *filp)
{

	printk("pcd_release\n");
	return 0;
}
int pcd_open(struct inode *inode,struct file *filp)
{
	printk("pcd_open\n");
	return 0;
}
struct file_operations pcd_fops=
{
	.open=pcd_open,
	.write=pcd_write,
	.read=pcd_read,
	.llseek=pcd_llseek,
	.release=pcd_release,
	.owner=THIS_MODULE
};

static int __init cdev_int_add(void)
{
	int ret;
	/* dynamically allocated a device number */
	ret =alloc_chrdev_region(&device_number,0,1,"pcd_dr");
	if(ret<0)
	{
		goto out;
	}
	printk("%s  major = %d minor =%d \n",__func__,MAJOR(device_number),MINOR(device_number));

	/* initlize cdev struct with fops*/
	cdev_init(&pcd_dev,&pcd_fops);

	/* register cdev  structure with VFS*/
	pcd_dev.owner=THIS_MODULE;
	ret=cdev_add(&pcd_dev,device_number,1);
	if(ret<0)
	{
		goto unreg_chrdev;
	}

	/* create device class under /sys/class */// creating the sys/class dirtory my_devv floader creating automaetic
	class_pcd =class_create(THIS_MODULE,"my_devv");

	if(IS_ERR(class_pcd))
	{
		ret=PTR_ERR(class_pcd);
		goto cdev_del;
	}


	/* populated the sysfs with device informstion */
	device_pcd = device_create(class_pcd,NULL,device_number,NULL,"pcd_o");// creating sys/class/my_devv/ drctory pcd_o folder all inforamtion avilable here for uevent other also  
	if(IS_ERR(device_pcd))
	{
		ret = PTR_ERR(device_pcd);
		goto device_pcd;
	}

	printk("init call for cdev int add\n");
	/* after dev/pcd_o file give permsiion read or right */
	return 0;

device_pcd:
	class_destroy(class_pcd);

cdev_del:
	cdev_del(&pcd_dev);

unreg_chrdev:
	unregister_chrdev_region(device_number,1);

out:
	return ret;
}

static void __exit cdev_int_add_exit(void)
{
	device_destroy(class_pcd,device_number);
	class_destroy(class_pcd);
	cdev_del(&pcd_dev);
	unregister_chrdev_region(device_number,1);
	printk("exit for cdev init add\n");
}

module_init(cdev_int_add);
module_exit(cdev_int_add_exit);

MODULE_DESCRIPTION("DIGAMBAR CDEV INIT ADD");
MODULE_LICENSE("GPL");

