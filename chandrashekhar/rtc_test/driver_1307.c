#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/rtc.h>
#include <linux/bcd.h>
#define DS1307_ADDRESS 0x68


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


static struct i2c_client *ds1307_client;

static int ds1307_read_time(struct device *dev, struct rtc_time *tm)
{

struct i2c_client *client = to_i2c_client(dev);

	unsigned char addr = DS1307_REG_SECS;
	unsigned char buf[7];

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
	if ((i2c_transfer(client->adapter, &msgs[0], 2)) != 2) {
		dev_err(&client->dev, "%s: read error\n", __func__);
		return -EIO;
	}

	tm->tm_sec	= bcd2bin(buf[0]&0x7F);
	tm->tm_min	= bcd2bin(buf[1]&0x7F);
	tm->tm_hour	= bcd2bin(buf[2]&0x3F);
	tm->tm_wday	= bcd2bin(buf[3]&0x07);
	tm->tm_mday	= bcd2bin(buf[4]&0x3F);
	tm->tm_mon	= bcd2bin(buf[5]&0x1F) - 1;
	tm->tm_year	= bcd2bin(buf[6]) + 100;

	return 0;
#if 0	
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

	return 0;
#endif	
}

static int ds1307_set_time(struct device *dev, struct rtc_time *tm)
{
struct i2c_client *client = to_i2c_client(dev);
	unsigned char buf[8];

	struct i2c_msg msg = {
		.addr = client->addr,
		.len = 8,
		.buf = buf,	/* write time/date */
	};

	buf[0] = DS1307_REG_SECS;
	buf[1] = bin2bcd(tm->tm_sec);
	buf[2] = bin2bcd(tm->tm_min);
	buf[3] = bin2bcd(tm->tm_hour);
	buf[4] = bin2bcd(tm->tm_wday);
	buf[5] = bin2bcd(tm->tm_mday);
	buf[6] = bin2bcd(tm->tm_mon + 1);
	buf[7] = bin2bcd(tm->tm_year - 100);

	/* write time/date registers */
	if ((i2c_transfer(client->adapter, &msg, 1)) != 1) {
		dev_err(&client->dev, "%s: write error\n", __func__);
		return -EIO;
	}

	return 0;

#if 0

	struct i2c_client *client = to_i2c_client(dev);
	struct i2c_msg msg;
	unsigned char buf[7];

	buf[0] = bin2bcd(tm->tm_sec);
	buf[1] = bin2bcd(tm->tm_min);
	buf[2] = bin2bcd(tm->tm_hour);
	buf[3] = bin2bcd(tm->tm_wday);
	buf[4] = bin2bcd(tm->tm_mday);
	buf[5] = bin2bcd(tm->tm_mon + 1);
	buf[6] = bin2bcd(tm->tm_year - 100);

	msg.addr = client->addr;
	msg.flags = 0;
	msg.buf = buf;
	msg.len = sizeof(buf);

	return i2c_transfer(client->adapter, &msg, 1);
#endif
}

static const struct rtc_class_ops ds1307_rtc_ops = {
	.read_time = ds1307_read_time,
	.set_time = ds1307_set_time,
};

static int ds1307_probe(struct i2c_client *client,const struct i2c_device_id *id)
{
	struct rtc_device *rtc;
	rtc=kzalloc(sizeof(struct rtc_device), GFP_KERNEL);
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
		return -ENODEV;

	rtc = devm_rtc_device_register(&client->dev,"ds1307_my", &ds1307_rtc_ops,THIS_MODULE);
	if (IS_ERR(rtc)) {
	dev_err(&client->dev, "Failed to register RTC device\n");
		return PTR_ERR(rtc);
	}
	i2c_set_clientdata(client, rtc);

	//ds1307_client = client;

	return 0;
}
static const struct of_device_id ds1307_of_match[] = {
	{
		.compatible = "dallas,ds1307",
	},
	{}
};
MODULE_DEVICE_TABLE(of, ds1307_of_match);

static const struct i2c_device_id ds1307_id[] = {
	{ "ds1307", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, ds1307_id);

static struct i2c_driver ds1307_driver = {
.driver = {
	.name = "ds1307_custom",
//	.owner = THIS_MODULE,
	.of_match_table = ds1307_of_match,
	},
	.probe = ds1307_probe,
	.id_table = ds1307_id,
};

module_i2c_driver(ds1307_driver);

MODULE_AUTHOR("Softnautics");
MODULE_DESCRIPTION("RTC DS1307 driver");
MODULE_LICENSE("GPL");
