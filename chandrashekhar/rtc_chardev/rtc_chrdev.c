#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/err.h>
#include<linux/kdev_t.h>
#include<linux/slab.h>
#include<linux/cdev.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include<linux/device.h>
#include <linux/rtc.h>
#include <linux/bcd.h>

#define MAX_SIZE 1024

#define I2C_BUS_AVAILABLE	1  
#define SLAVE_DEVICE_NAME  "DS1307"
#define DS1307_SLAVE_ADDRESS	0x68



#define DS1307_REG_SECS		0x00	/* 00-59 */
#	define DS1307_BIT_CH		0x80
#define DS1307_REG_MIN		0x01	/* 00-59 */

#define DS1307_REG_HOUR		0x02	/* 00-23, or 1-12{am,pm} */
#	define DS1307_BIT_12HR		0x40	/* in REG_HOUR */
#	define DS1307_BIT_PM		0x20	/* in REG_HOUR */

#define DS1307_REG_WDAY		0x03	/* 01-07 */

#define DS1307_REG_MDAY		0x04	/* 01-31 */
#define DS1307_REG_MONTH	0x05	/* 01-12 */

#define DS1307_REG_YEAR		0x06	/* 00-99 */

#define DS1307_REG_CONTROL	0x07		/* or ds1338 */
#	define DS1307_BIT_OUT		0x80
#	define DS1338_BIT_OSF		0x20
#	define DS1307_BIT_SQWE		0x10
#	define DS1307_BIT_RS1		0x02
#	define DS1307_BIT_RS0		0x01


static int __init ds1307_entry(void);
static void __exit ds1307_exit(void);
dev_t ds1307_dev=0;
static struct class *ds1307_class;
static struct cdev ds1307_cdev;
uint8_t *tempbuff,*kbuff;
struct rtc_time *tm;
static struct i2c_client *ds1307_client;


static int ds1307_open(struct inode *inode, struct file *file);
static int ds1307_release(struct inode *inode, struct file *file);
static ssize_t ds1307_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t ds1307_write(struct file *filp, const char *buf, size_t len, loff_t * off);

static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = ds1307_read,
        .write          = ds1307_write,
        .open           = ds1307_open,
        .release        = ds1307_release,
};

static const struct i2c_device_id ds1307_id[] = {
		{ SLAVE_DEVICE_NAME, 0 },
		{ }
};
MODULE_DEVICE_TABLE(i2c, ds1307_id);

static struct i2c_driver ds1307_driver = {
	.driver = {
		.name = SLAVE_DEVICE_NAME,
		.owner = THIS_MODULE
	}
};


static int ds1307_open(struct inode *inode, struct file *file)
{
	//printk(KERN_DEBUG "Here I am: %s:%i\n", __FUNCTION__, __LINE__);
	pr_info("device file opened......!!!\n");
	return 0;

}

static int ds1307_release(struct inode *inode, struct file *file)
{
	//printk(KERN_DEBUG "Here I am: %s:%i\n", __FUNCTION__, __LINE__);
	pr_info("device file closed......!!!\n");
	return 0;
}

static ssize_t  ds1307_read(struct file *filp, char __user *buf, size_t len,loff_t * off)
{
	//printk(KERN_DEBUG "Here I am: %s:%i\n", __FUNCTION__, __LINE__);

	s32 sec, min, hour, day, mon, year,date,ret,s,m,hr,d,mn,y;
	int to_copy;

	static char buff[30];


   	 sec = i2c_smbus_read_byte_data(ds1307_client, DS1307_REG_SECS);
	//printk(KERN_DEBUG "Here I am: %s:%i:sec=%d\n", __FUNCTION__, __LINE__,sec);
    	min = i2c_smbus_read_byte_data(ds1307_client, DS1307_REG_MIN);
	//printk(KERN_DEBUG "Here I am: %s:%i:min=%d\n", __FUNCTION__, __LINE__,min);
    	hour = i2c_smbus_read_byte_data(ds1307_client, DS1307_REG_HOUR);
	//printk(KERN_DEBUG "Here I am: %s:%i:hr=%d\n", __FUNCTION__, __LINE__,hour);
    	day = i2c_smbus_read_byte_data(ds1307_client, DS1307_REG_WDAY);
	//printk(KERN_DEBUG "Here I am: %s:%i:day=%d\n", __FUNCTION__, __LINE__,day);
    	date = i2c_smbus_read_byte_data(ds1307_client, DS1307_REG_MDAY);
	//printk(KERN_DEBUG "Here I am: %s:%i:date=%d\n", __FUNCTION__, __LINE__,date);
    	mon = i2c_smbus_read_byte_data(ds1307_client, DS1307_REG_MONTH);
	//printk(KERN_DEBUG "Here I am: %s:%i:mon=%d\n", __FUNCTION__, __LINE__,mon);
    	year = i2c_smbus_read_byte_data(ds1307_client, DS1307_REG_YEAR);
	//printk(KERN_DEBUG "Here I am: %s:%i:year=%d\n", __FUNCTION__, __LINE__,year);

	s = bcd2bin(sec & 0x7F);
	//printk(KERN_DEBUG "Here I am: %s:%i:ret_sec=%d\n", __FUNCTION__, __LINE__,s);
	m = bcd2bin(min & 0x7F);
	//printk(KERN_DEBUG "Here I am: %s:%i:ret_min=%d\n", __FUNCTION__, __LINE__,m);
	hr = bcd2bin(hour & 0x3F);
	//printk(KERN_DEBUG "Here I am: %s:%i:ret_hour=%d\n", __FUNCTION__, __LINE__,hr);

	
	ret = bcd2bin(day & 0x07);
	//printk(KERN_DEBUG "Here I am: %s:%i:ret_day=%d\n", __FUNCTION__, __LINE__,ret);

	d = bcd2bin(date & 0x3F);
	//printk(KERN_DEBUG "Here I am: %s:%i:ret_mday=%d\n", __FUNCTION__, __LINE__,d);
	mn = bcd2bin(mon & 0x1F);
	//printk(KERN_DEBUG "Here I am: %s:%i:ret_month=%d\n", __FUNCTION__, __LINE__,mn);
	y = bcd2bin(year);
	//printk(KERN_DEBUG "Here I am: %s:%i:ret_year=%d\n", __FUNCTION__, __LINE__,y);

	to_copy = min(sizeof(buff), len);


	snprintf(buff, sizeof(buff), "time:%d:%d:%d date:%d:%d:%d\n", hr, m,s,d,mn,y);		
	printk(KERN_DEBUG "time:%d:%d:%d date:%d:%d:%d\n", hr,m,s,d,mn,y);
		
	//ret = copy_to_user(buf,&buff,sizeof(buff));
	ret = copy_to_user(buf,&buff,sizeof(buff));
	if (ret)
	{	
	printk(KERN_DEBUG "Here I am: %s:%i\n", __FUNCTION__, __LINE__);
		return -EFAULT;
	}

	
	return ret;
	
}

static ssize_t  ds1307_write(struct file *filp, const char *buf, size_t len, loff_t * off)
{
	//printk(KERN_DEBUG "Here I am: %s:%i\n", __FUNCTION__, __LINE__);
	char hour_str[3], min_str[3], sec_str[3], date_str[3], month_str[3], year_str[3];
	int hour, min, sec, date, month, year;
	
	if(copy_from_user(tempbuff,buf,len))
	{
		pr_err("data error:ERROR!\n");
	}
	pr_info("data_write:DONE!\n");
	printk(KERN_DEBUG "Here I am: tempbuff=%s,len=%d\n", tempbuff,len);


	
	// Extracting components from the input string
	strncpy(hour_str, tempbuff, 2);
	hour_str[2] = '\0';
	strncpy(min_str, tempbuff + 2, 2);
	min_str[2] = '\0';
	strncpy(sec_str, tempbuff + 4, 2);
	sec_str[2] = '\0';
	strncpy(date_str, tempbuff + 6, 2);
	date_str[2] = '\0';
	strncpy(month_str, tempbuff + 8, 2);
	month_str[2] = '\0';
	strncpy(year_str, tempbuff + 10, 2);
	year_str[2] = '\0';

	// Convert the extracted components to integers
	sscanf(hour_str, "%d", &hour);
	sscanf(min_str, "%d", &min);
	sscanf(sec_str, "%d", &sec);
	sscanf(date_str, "%d", &date);
	sscanf(month_str, "%d", &month);
	sscanf(year_str, "%d", &year);

	// Print the extracted components
	printk(KERN_INFO "Hour: %02d\n", hour);
	printk(KERN_INFO "Minutes: %02d\n", min);
	printk(KERN_INFO "Seconds: %02d\n", sec);
	printk(KERN_INFO "Date: %02d\n", date);
	printk(KERN_INFO "Month: %02d\n", month);
	printk(KERN_INFO "Year: %02d\n", year);
	
	
	i2c_smbus_write_byte_data(ds1307_client,DS1307_REG_SECS, bin2bcd(sec));
	i2c_smbus_write_byte_data(ds1307_client,DS1307_REG_MIN, bin2bcd(min));
	i2c_smbus_write_byte_data(ds1307_client,DS1307_REG_HOUR, bin2bcd(hour));
	i2c_smbus_write_byte_data(ds1307_client,DS1307_REG_WDAY, bin2bcd(0));
	i2c_smbus_write_byte_data(ds1307_client,DS1307_REG_MDAY, bin2bcd(date));
	i2c_smbus_write_byte_data(ds1307_client,DS1307_REG_MONTH, bin2bcd(month));
	i2c_smbus_write_byte_data(ds1307_client,DS1307_REG_YEAR, bin2bcd(year));

	return len;



}

/*
** I2C Board Info strucutre
*/
static struct i2c_board_info ds1307_info = {
        I2C_BOARD_INFO("rtc-ds1307",0x68)
    };

static int __init ds1307_entry(void)
{
	int reg_val,temp_ch;
	int ret;
	static struct i2c_adapter *ds1307_adapter     = NULL;
	//printk(KERN_INFO "entry function of basic char_dev code\n");

	
	if(alloc_chrdev_region(&ds1307_dev,0,1,"ds_1307")<0)
	{
		pr_info("Cannot allocate major number\n");
                return -1;
        }
        pr_info("Major = %d Minor = %d \n",MAJOR(ds1307_dev), MINOR(ds1307_dev));

	if(IS_ERR(ds1307_class =class_create(THIS_MODULE, "ds1307_class")))
	{
		pr_info("Cannot create the struct class\n");
        	goto r_class;
	}
	if(IS_ERR(device_create(ds1307_class,NULL,ds1307_dev,NULL,"ds1307_device")))
	{
		pr_info("Cannot create the device\n");
        	goto r_device;
	}
	cdev_init(&ds1307_cdev,&fops);

	if((cdev_add(&ds1307_cdev,ds1307_dev,1)) < 0){
            pr_err("Cannot add the device to the system\n");
            goto r_dev;
        }
	if((tempbuff = kmalloc(MAX_SIZE , GFP_KERNEL)) == 0)
	{
        	pr_info("Cannot allocate memory in kernel\n");
        	goto r_device;
        }
       
    	ds1307_adapter     = i2c_get_adapter(I2C_BUS_AVAILABLE);

   	if(ds1307_adapter != NULL) {
		ds1307_client = i2c_new_client_device(ds1307_adapter, &ds1307_info);
		if(ds1307_client != NULL) {
			if(i2c_add_driver(&ds1307_driver) != -1) {
				ret = 0;
			}
			else
				printk("Can't add driver...\n");
		} 
	} 
        i2c_put_adapter(ds1307_adapter);
        pr_info("Device Driver Insert...Done!!!\n");

	reg_val = i2c_smbus_read_byte_data(ds1307_client, DS1307_REG_SECS);
	printk("reg_val: 0x%x\n", reg_val);
	temp_ch=reg_val>>7;
	if(temp_ch==0)
	{
		i2c_smbus_write_byte_data(ds1307_client, DS1307_REG_SECS, 0x00);
	}
		
	
	return 0;

r_dev:
	device_destroy(ds1307_class,ds1307_dev);

r_device:
	class_destroy(ds1307_class);
r_class:
	unregister_chrdev_region(ds1307_dev,1);
	return -1;	
}

static void __exit  ds1307_exit(void)
{
	i2c_unregister_device(ds1307_client);
	i2c_del_driver(&ds1307_driver);
	device_destroy(ds1307_class,ds1307_dev);
	class_destroy(ds1307_class);
	cdev_del(&ds1307_cdev);
	unregister_chrdev_region(ds1307_dev,1);
	printk(KERN_INFO "exit function of basic char_dev code\n");


}	
module_init (ds1307_entry);
module_exit (ds1307_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("chandrashekhar");




