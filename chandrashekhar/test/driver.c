

#include <linux/i2c.h>
#include <linux/bcd.h>
#include <linux/rtc.h>
#include <linux/slab.h>
#include <linux/module.h>

#define MAX_SIZE 1024

#define I2C_BUS_AVAILABLE	1  
#define SLAVE_DEVICE_NAME  "DS1307"
#define I2C_BUS_AVAILABLE	1		/* The I2C Bus available on the raspberry */
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


static struct i2c_driver ds1307_driver;
static int ds1307_rtc_read_time(struct device *dev, struct rtc_time *tm);
static int ds1307_rtc_set_time(struct device *dev, struct rtc_time *tm);


struct rtc_device *rtc;

static const struct rtc_class_ops ds1307_rtc_ops = {
	.read_time	= ds1307_rtc_read_time,
	.set_time	= ds1307_rtc_set_time,
};

static int ds1307_rtc_read_time(struct device *dev, struct rtc_time *tm)
{

	printk(KERN_DEBUG "Here I am: %s:%i\n", __FUNCTION__, __LINE__);
	struct i2c_client *client = to_i2c_client(dev);
	struct i2c_msg msg[2];
	unsigned char buf[7];
	int ret;

	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].buf = buf;
	msg[0].len = 1;

	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].buf = buf;
	msg[1].len = sizeof(buf);

	ret = i2c_transfer(client->adapter, msg, 2);
	if (ret != 2) {
	dev_err(dev, "Error reading time from RTC\n");
	return ret < 0 ? ret : -EIO;
	}

	tm->tm_sec = bcd2bin(buf[0] & 0x7F);
	tm->tm_min = bcd2bin(buf[1] & 0x7F);
	tm->tm_hour = bcd2bin(buf[2] & 0x3F);
	tm->tm_mday = bcd2bin(buf[4] & 0x3F);
	tm->tm_mon = bcd2bin(buf[5] & 0x1F) - 1;
	tm->tm_year = bcd2bin(buf[6]) + 100;
	return rtc_valid_tm(tm);
}
static int ds1307_rtc_set_time(struct device *dev, struct rtc_time *tm)
{
	printk(KERN_DEBUG "Here I am: %s:%i\n", __FUNCTION__, __LINE__);
	struct i2c_client *client = to_i2c_client(dev);
	struct i2c_msg msg;
	unsigned char buf[8];

	buf[0] = 0;
	buf[1] = bin2bcd(tm->tm_sec);
	buf[2] = bin2bcd(tm->tm_min);
	buf[3] = bin2bcd(tm->tm_hour);
	buf[4] = bin2bcd(tm->tm_wday);
	buf[5] = bin2bcd(tm->tm_mday);
	buf[6] = bin2bcd(tm->tm_mon + 1);
	buf[7] = bin2bcd(tm->tm_year - 100);

	msg.addr = client->addr;
	msg.flags = 0;
	msg.buf = buf;
	msg.len = sizeof(buf);

	return i2c_transfer(client->adapter, &msg, 1);
}



static int ds1307_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{

	int err = 0;

	dev_dbg(&client->dev, "%s\n", __func__);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
		return -ENODEV;

	rtc = kzalloc(sizeof(struct rtc_device), GFP_KERNEL);
	if (!rtc)
		return -ENOMEM;


	printk(KERN_DEBUG "Here I am: %s:%i\n", __FUNCTION__, __LINE__);

	rtc = devm_rtc_device_register(&client->dev,ds1307_driver.driver.name, &ds1307_rtc_ops, THIS_MODULE);

	if (IS_ERR(rtc)) {
		err = PTR_ERR(rtc);
		goto exit_kfree;
	}
	i2c_set_clientdata(client, rtc);
	printk(KERN_DEBUG "Here I am: %s:%i\n", __FUNCTION__, __LINE__);
	return 0;

exit_kfree:
	kfree(rtc);

	return err;
}
#if 0
static int ds1307_remove(struct i2c_client *client)
{
	

	if (rtc)
		//rtc_device_unregister(ds1307->rtc);
		devm_rtc_device_unregister(&client->dev,rtc);

	kfree(rtc);

	return 0;
}
#endif
static const struct i2c_device_id ds1307_id[] = {
	{ "ds1307", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, ds1307_id);

static struct i2c_driver ds1307_driver = {
	.driver		= {
		.name	= "rtc-ds1307_cstm",
	},
	.probe		= ds1307_probe,
	//.remove		= ds1307_remove,
	.id_table	= ds1307_id,
};

static int __init ds1307_init(void)
{
	printk(KERN_DEBUG "Here I am: %s:%i\n", __FUNCTION__, __LINE__);
	return i2c_add_driver(&ds1307_driver);
}

static void __exit ds1307_exit(void)
{
	printk(KERN_DEBUG "Here I am: %s:%i\n", __FUNCTION__, __LINE__);
	i2c_del_driver(&ds1307_driver);
}

MODULE_AUTHOR("chandrashekhar");
MODULE_DESCRIPTION("basic rtc driver for ds1307");
MODULE_LICENSE("GPL");


module_init(ds1307_init);
module_exit(ds1307_exit);

