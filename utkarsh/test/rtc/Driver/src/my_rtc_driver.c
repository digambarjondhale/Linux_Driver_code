/*
 * write basic device driver for RTC which must be able to update and read date and time
*/

/****************INCLUDE SECTION*********************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/err.h>
#include <linux/kdev_t.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/rtc.h>
#include <linux/types.h>
#include <linux/bcd.h>

/****************MACRO DEFINITION********************/
#define MAX_SIZE 1024

#define I2C_BUS_AVAILABLE	1
#define SLAVE_DEVICE_NAME	"DS1307"
#define I2C_BUS_AVAILABLE	1	/* The I2C Bus available on the raspberry */
#define DS1307_SLAVE_ADDRESS	0x68

#define DS1307_REG_SECS		0x00	/* 00-59 */
#define DS1307_BIT_CH		0x80
#define DS1307_REG_MIN		0x01	/* 00-59 */

#define DS1307_REG_HOUR		0x02	/* 00-23, or 1-12{am,pm} */
#define DS1307_BIT_12HR		0x40	/* in REG_HOUR */
#define DS1307_BIT_PM		0x20	/* in REG_HOUR */

#define DS1307_REG_WDAY		0x03	/* 01-07 */

#define DS1307_REG_MDAY		0x04	/* 01-31 */
#define DS1307_REG_MONTH	0x05	/* 01-12 */

#define DS1307_REG_YEAR		0x06	/* 00-99 */

#define DS1307_REG_CONTROL	0x07
#define DS1307_BIT_OUT		0x80
#define DS1338_BIT_OSF		0x20
#define DS1307_BIT_SQWE		0x10
#define DS1307_BIT_RS1		0x02
#define DS1307_BIT_RS0		0x01

/****************GLOBAL SECTION*********************/
dev_t ds1307_dev = 0;
static struct class *ds1307_class;
static struct cdev ds1307_cdev;
uint8_t *tempbuff;

static struct i2c_client *ds1307_client;
struct rtc_time *tm;
struct rtc_device *rtc;
struct device *rtc_dev;

/****************FUNCTION PROTOTYPES****************/
static int __init ds1307_entry(void);
static void __exit ds1307_exit(void);
static int ds1307_open(struct inode *inode, struct file *file);
static int ds1307_release(struct inode *inode, struct file *file);
static ssize_t ds1307_read(struct file *filp, char __user *buf, size_t len, loff_t * off);
static ssize_t ds1307_write(struct file *filp, const char *buf, size_t len, loff_t * off);
static int ds1307_rtc_set_time(struct device *dev, struct rtc_time *tm);
static int ds1307_rtc_read_time(struct device *dev, struct rtc_time *tm);

/* file_operations structure */
static struct file_operations fops = {
	.owner          = THIS_MODULE,
	.read           = ds1307_read,
	.write          = ds1307_write,
	.open           = ds1307_open,
	.release        = ds1307_release,
};

/* array of I2C device IDs */
static const struct i2c_device_id ds1307_id[] = {
	{ SLAVE_DEVICE_NAME, 0 },
	{ }
};

/* I2C driver structure */
static struct i2c_driver ds1307_driver = {
	.id_table = ds1307_id,
	.driver = {
		.name = "my_rtc",
		.owner = THIS_MODULE
	},
};

/* structure specifying operations that RTC class should perform */
static const struct rtc_class_ops ds1307_rtc_ops = {
	.read_time = ds1307_rtc_read_time,
	.set_time = ds1307_rtc_set_time
};

/*
 * function: ds1307_open()
 *
 * This function is called when the device file is opened.
 *
 * @param struct inode *inode: Represents the inode associated with the opened device file.
 * @param struct file *file: Represents the file instance that corresponds to the opened device file.
 *
 * @return 0: On successful opening of the device file.
 *
 */
static int ds1307_open(struct inode *inode, struct file *file){
	pr_info("device file opened......!!!\n");
	return 0;

}

/*
 * function: ds1307_release()
 *
 * This function is called when the device file is closed.
 *
 * @param struct inode *inode: Represents the inode associated with the opened device file.
 * @param struct file *file: Represents the file instance that corresponds to the opened device file.
 *
 * @return 0: On successful closing of the device file.
 *
 */
static int ds1307_release(struct inode *inode, struct file *file){
	pr_info("device file closed......!!!\n");
	return 0;
}

/*
 * function: ds1307_read()
 *
 * This function is called when data is read from the device file.
 *
 * @param struct file *filp: Represents the file instance that corresponds to the opened device file.
 * @param char __user *buf: Pointer to the user space buffer to store the read data.
 * @param size_t len: Number of bytes to read.
 * @param loff_t *off: Pointer to the file offset.
 *
 * @return ret: Number of bytes read.
 * @return -EFAULT: failed to read data
 *
 */
static ssize_t ds1307_read(struct file *filp, char __user *buf, size_t len, loff_t * off){
	s32 sec, min, hour, day, mon, year, date, ret, s, m, hr, d, mn, y, wday;
	static char buff[30];
	/* read a single byte of data from the DS1307 RTC's register */
	sec = i2c_smbus_read_byte_data(ds1307_client, DS1307_REG_SECS);
	min = i2c_smbus_read_byte_data(ds1307_client, DS1307_REG_MIN);
	hour = i2c_smbus_read_byte_data(ds1307_client, DS1307_REG_HOUR);
	date = i2c_smbus_read_byte_data(ds1307_client, DS1307_REG_MDAY);
	mon = i2c_smbus_read_byte_data(ds1307_client, DS1307_REG_MONTH);
	year = i2c_smbus_read_byte_data(ds1307_client, DS1307_REG_YEAR);
	/* conversion of BCD value to binary */
	s = bcd2bin(sec & 0x7F);
	m = bcd2bin(min & 0x7F);
	hr = bcd2bin(hour & 0x3F);
	wday = bcd2bin(day & 0x07);
	d = bcd2bin(date & 0x3F);
	mn = bcd2bin(mon & 0x1F);
	y = bcd2bin(year);
	snprintf(buff, sizeof(buff), "time:%d:%d:%d date:%d:%d:%d\n", hr, m, s, d, mn, y);
	printk(KERN_DEBUG "time:%d:%d:%d date:%d:%d:%d\n", hr, m, s, d, mn, y);
	ret = copy_to_user(buf, &buff, sizeof(buff));
	if(ret){
		printk(KERN_DEBUG "Here I am: %s:%i\n", __FUNCTION__, __LINE__);
		return -EFAULT;
	}
	return ret;
}

/*
 * function: ds1307_write()
 *
 * This function is called when data is written to the device file.
 *
 * @param struct file *filp: Represents the file instance that corresponds to the opened device file.
 * @param const char *buf: Pointer to the user space buffer containing the data to write.
 * @param size_t len: Number of bytes to write.
 * @param loff_t *off: Pointer to the file offset.
 *
 * @return len: Number of bytes written.
 *
 */
static ssize_t ds1307_write(struct file *filp, const char *buf, size_t len, loff_t * off){
	char hour_str[3], min_str[3], sec_str[3], date_str[3], month_str[3], year_str[3];
	int hour, min, sec, date, month, year;
	if(copy_from_user(tempbuff, buf, len)){
		pr_err("data error:ERROR!\n");
	}
	pr_info("data_write:DONE!\n");
	printk(KERN_DEBUG "Here I am: tempbuff = %s, len = %d\n", tempbuff, len);
	/* Extracting components from the input string */
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
	/* Convert the extracted components to integers */
	sscanf(hour_str, "%d", &hour);
	sscanf(min_str, "%d", &min);
	sscanf(sec_str, "%d", &sec);
	sscanf(date_str, "%d", &date);
	sscanf(month_str, "%d", &month);
	sscanf(year_str, "%d", &year);
	/* Print the extracted components */
	printk(KERN_INFO "Hour: %02d\n", hour);
	printk(KERN_INFO "Minutes: %02d\n", min);
	printk(KERN_INFO "Seconds: %02d\n", sec);
	printk(KERN_INFO "Date: %02d\n", date);
	printk(KERN_INFO "Month: %02d\n", month);
	printk(KERN_INFO "Year: %02d\n", year);
	/* writing a single byte of data to the DS1307 RTC's register */
	i2c_smbus_write_byte_data(ds1307_client, DS1307_REG_SECS, bin2bcd(sec));
	i2c_smbus_write_byte_data(ds1307_client, DS1307_REG_MIN, bin2bcd(min));
	i2c_smbus_write_byte_data(ds1307_client, DS1307_REG_HOUR, bin2bcd(hour));
	i2c_smbus_write_byte_data(ds1307_client, DS1307_REG_MDAY, bin2bcd(date));
	i2c_smbus_write_byte_data(ds1307_client, DS1307_REG_MONTH, bin2bcd(month));
	i2c_smbus_write_byte_data(ds1307_client, DS1307_REG_YEAR, bin2bcd(year));

	return len;
}

/*
 * function: ds1307_rtc_read_time()
 *
 * This function reads the time and date registers from the DS1307 RTC
 * and stores the converted values in the rtc_time structure.
 *
 * @param struct device *dev: Pointer to the struct device representing the RTC device
 * @param struct rtc_time *tm: Pointer to the struct rtc_time where the time and date will be stored
 *
 * @return 0: on success reading time
 * @return -EIO: read error
 *
*/
static int ds1307_rtc_read_time(struct device *dev, struct rtc_time *tm){
	struct i2c_client *client = to_i2c_client(dev);
	unsigned char addr = DS1307_REG_SECS;
	unsigned char buf[7];

	if((dev->driver) == NULL)
		pr_info("Dev is NULL\n");

	struct i2c_msg msgs[] = {
		{/* setup read addr */
			.addr = client->addr,
			.len = 1,
			.buf = &addr
		},
		{/* read time/date */
			.addr = client->addr,
			.flags = I2C_M_RD,
			.len = 7,
			.buf = buf
		},
	};

	/* read time/date registers */
	if((i2c_transfer(client->adapter, &msgs[0], 2)) != 2){
		dev_err(&client->dev, "%s: read error\n", __func__);
		return -EIO;
	}

	tm->tm_sec = bcd2bin(buf[0] & 0x7F);
	tm->tm_min = bcd2bin(buf[1] & 0x7F);
	tm->tm_hour = bcd2bin(buf[2] & 0x3F);
	tm->tm_mday = bcd2bin(buf[3] & 0x3F);
	tm->tm_mon = bcd2bin(buf[4] & 0x1F);
	tm->tm_year = bcd2bin(buf[5]);

	return 0;

}

/*
 * function: ds1307_rtc_set_time()
 *
 * This function sets the time and date registers in the DS1307 RTC
 * with the values provided in the rtc_time structure.
 *
 * @param struct device *dev: Pointer to the struct device representing the RTC device
 * @param struct rtc_time *tm: Pointer to the struct rtc_time where the time and date will be stored
 *
 * @return 0: on success setting time
 * @return -EIO: write error
*/
static int ds1307_rtc_set_time(struct device *dev, struct rtc_time *tm){
	struct i2c_client *client = to_i2c_client(dev);
	unsigned char buf[8];

	pr_info("set_rtc_time\n");
	struct i2c_msg msg = {
		.addr = client->addr,
		.len = 8,
		.buf = buf,	/* write time/date */
	};

	buf[0] = DS1307_REG_SECS;
	buf[1] = bin2bcd(tm->tm_sec);
	buf[2] = bin2bcd(tm->tm_min);
	buf[3] = bin2bcd(tm->tm_hour);
	buf[4] = bin2bcd(tm->tm_mday);
	buf[5] = bin2bcd(tm->tm_mon);
	buf[6] = bin2bcd(tm->tm_year);

	/* write time/date registers */
	if((i2c_transfer(client->adapter, &msg, 1)) != 1){
		dev_err(&client->dev, "%s: write error\n", __func__);
		return -EIO;
	}
	return 0;
}

/* I2C Board info structure */
static struct i2c_board_info ds1307_info = {
	I2C_BOARD_INFO("rtc-ds1307", 0x68)
};

/*
 * function: ds1307_entry()
 *
 * This function is called when the module is loaded into the kernel.
 *
 * Input: void
 *
 * @return 0: on successful loading of module.
 * @return -1: on failure while loading module.
 * @return -ENOMEM: memory allocation failed
 *
 */
static int __init ds1307_entry(void){
	int reg_val, temp_ch, ret;
	static struct i2c_adapter *ds1307_adapter = NULL;

	/* dynamically allocating major number */
	if(alloc_chrdev_region(&ds1307_dev, 0, 1, "my_ds_1307") < 0){
		pr_info("Cannot allocate major number\n");
		return -1;
	}
	pr_info("Major = %d Minor = %d \n", MAJOR(ds1307_dev), MINOR(ds1307_dev));

	/* create a device class */
	if(IS_ERR(ds1307_class = class_create(THIS_MODULE, "my_ds1307_class"))){
		pr_info("Cannot create the struct class\n");
		goto r_class;
	}

 	/* creating device and registering it with device class */
	if(IS_ERR(device_create(ds1307_class, NULL, ds1307_dev, NULL, "my_ds1307_dev"))){
		pr_info("Cannot create the device\n");
		goto r_device;
	}

	/* initialize the character device structure */
	cdev_init(&ds1307_cdev, &fops);

	/* add character device to the system */
	if((cdev_add(&ds1307_cdev, ds1307_dev, 1)) < 0){
		pr_err("Cannot add the device to the system\n");
		goto r_dev;
	}
	if((tempbuff = kmalloc(MAX_SIZE, GFP_KERNEL)) == 0){
		pr_info("Cannot allocate memory in kernel\n");
		goto r_device;
	}

	/* Get the I2C adapter for the DS1307 */
	ds1307_adapter = i2c_get_adapter(I2C_BUS_AVAILABLE);

	if(ds1307_adapter != NULL){
		/* Create a new I2C client device for the DS1307 RTC */
		ds1307_client = i2c_new_client_device(ds1307_adapter, &ds1307_info);
		if(ds1307_client != NULL){
			/* Register the DS1307 I2C driver with the Linux I2C subsystem */
			if(i2c_add_driver(&ds1307_driver) != -1){
				printk(KERN_DEBUG "Here I am: %s:%i\n", __FUNCTION__, __LINE__);
				ret = 0;
			}
			else
				printk("Can't add driver...\n");
		}
	}

	/* Release the reference to the I2C adapter obtained earlier */
	i2c_put_adapter(ds1307_adapter);
	reg_val = i2c_smbus_read_byte_data(ds1307_client, 0x0);
	printk("reg_val: 0x%x\n", reg_val);
	temp_ch = reg_val >> 7;
	if(temp_ch == 0){
		i2c_smbus_write_byte_data(ds1307_client, 0x00, 0x00);
	}
	printk(KERN_DEBUG "Here I am: %s:%i\n", __FUNCTION__, __LINE__);
	rtc = kzalloc(sizeof(struct rtc_device), GFP_KERNEL);
	if(!rtc)
		return -ENOMEM;
#if 1
	/* Register the RTC device with the kernel's RTC framework */
	rtc = devm_rtc_device_register(&ds1307_client->dev, "my_ds1307", &ds1307_rtc_ops, THIS_MODULE);
	if(IS_ERR(rtc)){
		printk(KERN_DEBUG "Here I am: %s:%i\n", __FUNCTION__, __LINE__);
		return PTR_ERR(rtc);
	}
#endif
	pr_info("Device Driver Insert...Done!!!\n");

	return 0;

r_dev:
	device_destroy(ds1307_class, ds1307_dev);

r_device:
	class_destroy(ds1307_class);
r_class:
	unregister_chrdev_region(ds1307_dev, 1);
	return -1;
}

/*
 * function: ds1307_exit()
 *
 * This function is called when the module is removed from the kernel.
 *
 * Input: void
 *
 * @return void
 *
 */
static void __exit  ds1307_exit(void){
	i2c_unregister_device(ds1307_client);
	i2c_del_driver(&ds1307_driver);
	device_destroy(ds1307_class, ds1307_dev);
	class_destroy(ds1307_class);
	cdev_del(&ds1307_cdev);
	unregister_chrdev_region(ds1307_dev, 1);
	printk(KERN_INFO "exit function of basic char_dev code\n");
}
module_init (ds1307_entry);
module_exit (ds1307_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("UV");
