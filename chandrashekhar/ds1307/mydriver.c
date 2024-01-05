#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/rtc.h>
#include <linux/bcd.h>

#define DS1307_REG_SECONDS 0x00
#define DS1307_REG_MINUTES 0x01
#define DS1307_REG_HOURS 0x02
#define DS1307_REG_DAY 0x03
#define DS1307_REG_MONTH 0x04
#define DS1307_REG_YEAR 0x05
#define DS1307_REG_CONTROL 0x07

static struct i2c_client *ds1307_client;

static int ds1307_read_time(struct device *dev, struct rtc_time *tm)
{
	u8 buf[7];
	int ret;
	printk(KERN_DEBUG "Here I am: %s:%i\n", __FUNCTION__, __LINE__);

	ret = i2c_smbus_read_i2c_block_data(ds1307_client, DS1307_REG_SECONDS, sizeof(buf), buf);
	if (ret < 0)
		return ret;

	tm->tm_sec = bcd2bin(buf[DS1307_REG_SECONDS] & 0x7F);
	tm->tm_min = bcd2bin(buf[DS1307_REG_MINUTES] & 0x7F);
	tm->tm_hour = bcd2bin(buf[DS1307_REG_HOURS] & 0x3F);
	tm->tm_mday = bcd2bin(buf[DS1307_REG_DAY] & 0x3F);
	tm->tm_mon = bcd2bin(buf[DS1307_REG_MONTH] & 0x1F) - 1;
	tm->tm_year = bcd2bin(buf[DS1307_REG_YEAR]) + 100;

	return rtc_valid_tm(tm);
}

static int ds1307_set_time(struct device *dev, struct rtc_time *tm)
{
	u8 buf[7];
	printk(KERN_DEBUG "Here I am: %s:%i\n", __FUNCTION__, __LINE__);

	buf[DS1307_REG_SECONDS] = bin2bcd(tm->tm_sec);
	buf[DS1307_REG_MINUTES] = bin2bcd(tm->tm_min);
	buf[DS1307_REG_HOURS] = bin2bcd(tm->tm_hour);
	buf[DS1307_REG_DAY] = bin2bcd(tm->tm_mday);
	buf[DS1307_REG_MONTH] = bin2bcd(tm->tm_mon + 1);
	buf[DS1307_REG_YEAR] = bin2bcd(tm->tm_year % 100);

	return i2c_smbus_write_i2c_block_data(ds1307_client, DS1307_REG_SECONDS, sizeof(buf), buf);
}

static const struct rtc_class_ops ds1307_rtc_ops = {
	.read_time = ds1307_read_time,
	.set_time = ds1307_set_time,
};

static int ds1307_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct rtc_device *rtc;
	int err;
	
	rtc = kzalloc(sizeof(struct rtc_device), GFP_KERNEL);
	if (!rtc)
		return -ENOMEM;
	//ds1307_client = client;
#if 0
	rtc = devm_rtc_device_register(&client->dev, "ds1307", &ds1307_rtc_ops, THIS_MODULE);
	if (IS_ERR(rtc)) {
		dev_err(&client->dev, "Failed to register RTC device\n");
		return PTR_ERR(rtc);
	}

	ret = rtc_register_device(rtc);
	if (ret) {
		dev_err(&client->dev, "Failed to register RTC class device\n");
	return ret;
	}
#endif
	rtc = devm_rtc_allocate_device(&client->dev);
	if (IS_ERR(rtc))
		return PTR_ERR(rtc);

	rtc->ops= &ds1307_rtc_ops;
	i2c_set_clientdata(client,rtc);


	err = devm_rtc_register_device(rtc);
	if (err)
		return err;


	return 0;
}

static const struct i2c_device_id ds1307_id[] = {
	{ "ds1307", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, ds1307_id);

static struct i2c_driver ds1307_driver = {
	.driver = {
		.name = "ds1307_cstm",
	},
	.probe = ds1307_probe,
	.id_table = ds1307_id,
};

module_i2c_driver(ds1307_driver);

MODULE_DESCRIPTION("RTC driver for DS1307");
MODULE_AUTHOR("Your Name");
MODULE_LICENSE("GPL");
