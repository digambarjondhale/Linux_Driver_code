#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/kdev_t.h>
#include<linux/uaccess.h>
#define DEV_MEM_SIZE_PCDEV_1 512
#define DEV_MEM_SIZE_PCDEV_2 512
#define DEV_MEM_SIZE_PCDEV_3 512
#define DEV_MEM_SIZE_PCDEV_4 512

char device_buffer_pcddev_1[DEV_MEM_SIZE_PCDEV_1];
char device_buffer_pcddev_2[DEV_MEM_SIZE_PCDEV_2];
char device_buffer_pcddev_3[DEV_MEM_SIZE_PCDEV_3];
char device_buffer_pcdev_4[DEV_MEM_SIZE_PCDEV_4];


/* device private data structure */

struct pcddev_private_data
{
	char *buffer;
	unsigned size;
	const char *serial_number;
	int permision;
	struct cdev cdev;
};
#define NO_OF_DEVICE 4
/* driver private data structure */


struct pcdrv_privte_data
{
	int total_device;
	dev_t device_number;
	struct class *class_pcd;
	struct device *device_pcd;
	struct pcddev_private_data pcddev_data[NO_OF_DEVICE];
};

struct pcdrv_privte_data pcd_private_driver={
	.total_device=NO_OF_DEVICE,
	.pcddev_data={
		[0]={
			.buffer=device_buffer_pcddev_1,
			.size=DEV_MEM_SIZE_PCDEV_1,
			.serial_number="DI123",
			.permision=0x11
		},

		[1]={
			.buffer=device_buffer_pcddev_2,
			.size=DEV_MEM_SIZE_PCDEV_2,
			.serial_number="DI456",
			.permision=0x11

		},

		[2]={
			.size=DEV_MEM_SIZE_PCDEV_3,
			.serial_number="DI789",
			.permision=0x11,
			.buffer=device_buffer_pcddev_3
		},

		[3]={.permision=0x11,
			.size=DEV_MEM_SIZE_PCDEV_4,
			.buffer=device_buffer_pcdev_4,
			.serial_number="DI1011"
		}
	}

};

loff_t pcd_llseek (struct file *flip, loff_t offset , int where)
{
	int DEV_MEM_SIZE;

	struct pcddev_private_data *driver = (struct pcddev_private_data *)flip->private_data;
	DEV_MEM_SIZE=driver->size;
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
	int DEV_MEM_SIZE;
	struct pcddev_private_data *driver=(struct pcddev_private_data *)filp->private_data;
	DEV_MEM_SIZE=driver->size;
	printk("befor read opration offset count %lld\n",*f_pos);
	/* adjust count */
	if((*f_pos+count)>DEV_MEM_SIZE)                                         
		count=DEV_MEM_SIZE - *f_pos;

	/*copy to user */
	if(copy_to_user(buff,driver->buffer + (*f_pos),count))
		printk("Error occuer in copy to user\n");


	/* update the offset */
	*f_pos +=count;
	printk("after read oprtion offset count %lld\n",*f_pos);

	return count;
}
ssize_t pcd_write (struct file *filp,const char __user *buff,size_t count, loff_t *f_pos)
{
	int DEV_MEM_SIZE;
	struct pcddev_private_data *driver =(struct pcddev_private_data *)filp->private_data;
	DEV_MEM_SIZE=driver->size;
	printk("befor write data offset %lld\n",*f_pos);
	/* adjust count */
	if((*f_pos+count) >DEV_MEM_SIZE)
		count=DEV_MEM_SIZE - *f_pos;

	/* copy from user */
	if(copy_from_user(driver->buffer + *f_pos,buff,count))
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
	struct pcddev_private_data *driver;
	int minor;
	minor=MINOR(inode->i_rdev);
	driver=container_of(inode->i_cdev,struct pcddev_private_data ,cdev);
	filp->private_data=driver;
	//ret cheak_permission(driver->permision,flip->f_mode);
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

	int i;
	/* dynamically allocated a device number */
	ret =alloc_chrdev_region(&pcd_private_driver.device_number,0,NO_OF_DEVICE,"pcd_dr");
	if(ret<0)
	{
		goto out;
	}
	/* create device class under /sys/class */// creating the sys/class dirtory my_devv floader creating automaetic
	pcd_private_driver.class_pcd =class_create(THIS_MODULE,"my_devv");

	if(IS_ERR(pcd_private_driver.class_pcd))
	{
		ret=PTR_ERR(pcd_private_driver.class_pcd);
		goto cdev_del;
	}
	for(i=0;i<4;i++)
	{
		printk("%s  major = %d minor =%d \n",__func__,MAJOR(pcd_private_driver.device_number+i),MINOR(pcd_private_driver.device_number+i));

		/* initlize cdev struct with fops*/
		cdev_init(&pcd_private_driver.pcddev_data[i].cdev,&pcd_fops);

		/* register cdev  structure with VFS*/
		pcd_private_driver.pcddev_data[i].cdev.owner=THIS_MODULE;
		ret=cdev_add(&pcd_private_driver.pcddev_data[i].cdev,pcd_private_driver.device_number+i,1);

		if(ret<0)
		{
			goto unreg_chrdev;
		}



		/* populated the sysfs with device informstion */
		pcd_private_driver.device_pcd = device_create(pcd_private_driver.class_pcd,NULL,pcd_private_driver.device_number+i,NULL,"pcd_o %d",i+1);// creating sys/class/my_devv/ drctory pcd_o folder all inforamtion avilable here for uevent other also  
		if(IS_ERR(pcd_private_driver.device_pcd))
		{
			ret = PTR_ERR(pcd_private_driver.device_pcd);
			goto device_pcd;
		}

		printk("init call for cdev int add\n");
		/* after dev/pcd_o file give permsiion read or right */
	}
	return 0;

device_pcd:
cdev_del:
	for(i=3;i>=0;i--)
	{
		device_destroy(pcd_private_driver.class_pcd,pcd_private_driver.device_number+i);
		cdev_del(&pcd_private_driver.pcddev_data[i].cdev);
	}
	class_destroy(pcd_private_driver.class_pcd);
unreg_chrdev:
	unregister_chrdev_region(pcd_private_driver.device_number,NO_OF_DEVICE);

out:
	return ret;
}

static void __exit cdev_int_add_exit(void)
{
	int i;
	for(i=0;i<NO_OF_DEVICE;i++)
	{
		device_destroy(pcd_private_driver.class_pcd,pcd_private_driver.device_number+i);
		cdev_del(&pcd_private_driver.pcddev_data[i].cdev);
	}
	class_destroy(pcd_private_driver.class_pcd);

	unregister_chrdev_region(pcd_private_driver.device_number,NO_OF_DEVICE);
	printk("exit for cdev init add\n");
}

module_init(cdev_int_add);
module_exit(cdev_int_add_exit);

MODULE_DESCRIPTION("DIGAMBAR CDEV INIT ADD");
MODULE_LICENSE("GPL");

