#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/rtc.h>
#include <linux/bcd.h>
#define DS1307_ADDRESS 0x68

static struct i2c_client *ds1307_client;

static int ds1307_read_time(struct device *dev, struct rtc_time *tm)
{
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
}

static int ds1307_set_time(struct device *dev, struct rtc_time *tm)
{
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

static const struct rtc_class_ops ds1307_rtc_ops = {
	.read_time = ds1307_read_time,
	.set_time = ds1307_set_time,
};

static int ds1307_probe(struct i2c_client *client,const struct i2c_device_id *id)
{
	struct rtc_device *rtc;

	rtc = devm_rtc_device_register(&client->dev,client->adapter->dev.of_node->full_name, &ds1307_rtc_ops,THIS_MODULE);
	if (IS_ERR(rtc)) {
	dev_err(&client->dev, "Failed to register RTC device\n");
		return PTR_ERR(rtc);
	}

	//ds1307_client = client;
	pr_info("probe is done\n");
	return 0;
}


static const struct i2c_device_id ds1307_id[] = {
	{ "ds1307", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, ds1307_id);

static struct i2c_driver ds1307_driver = {
.driver = {
	.name = "rtc-ds1307_custom",
	.owner = THIS_MODULE,
	},
	.probe = ds1307_probe,
	.id_table = ds1307_id,
};

module_i2c_driver(ds1307_driver);

MODULE_AUTHOR("Softnautics");
MODULE_DESCRIPTION("RTC DS1307 driver");
MODULE_LICENSE("GPL");
