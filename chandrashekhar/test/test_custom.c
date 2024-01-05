
#include <linux/bcd.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/property.h>
#include <linux/rtc/ds1307.h>
#include <linux/rtc.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/regmap.h>

enum ds_type {
	unknown_ds_type, /* always first and 0 */
	ds_1307,
	last_ds_type 
};

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
#	define DS1307_BIT_SQWE		0x10
#	define DS1307_BIT_RS1		0x02
#	define DS1307_BIT_RS0		0x01


struct ds1307 {
	enum ds_type		type;
	struct device		*dev;
	struct regmap		*regmap;
	const char		*name;
	struct rtc_device	*rtc;
};

struct chip_desc {
	
	u8			offset; /* register's offset */
	u8			century_reg;
	u8			century_enable_bit;
	u8			century_bit;
	u8			bbsqi_bit;
	const struct rtc_class_ops *rtc_ops;

};

static const struct chip_desc chips[last_ds_type];
static int ds1307_get_time(struct device *dev, struct rtc_time *t)
{
	struct ds1307   *ds1307 = dev_get_drvdata(dev);
        int             tmp, ret;
        const struct chip_desc *chip = &chips[ds1307->type];
        u8 regs[7];


        /* read the RTC date and time registers all at once */
        ret = regmap_bulk_read(ds1307->regmap, chip->offset, regs,
                               sizeof(regs));
        if (ret) {
                dev_err(dev, "%s error %d\n", "read", ret);
                return ret;
        }

        dev_dbg(dev, "%s: %7ph\n", "read", regs);


        tmp = regs[DS1307_REG_SECS];
        switch (ds1307->type) {
        case ds_1307:
                if (tmp & DS1307_BIT_CH)
                        return -EINVAL;
                break;
        default:
                break;
        }

        t->tm_sec = bcd2bin(regs[DS1307_REG_SECS] & 0x7f);
        t->tm_min = bcd2bin(regs[DS1307_REG_MIN] & 0x7f);
        tmp = regs[DS1307_REG_HOUR] & 0x3f;
        t->tm_hour = bcd2bin(tmp);
                t->tm_wday = bcd2bin(regs[DS1307_REG_WDAY] & 0x07) - 1;
        t->tm_mday = bcd2bin(regs[DS1307_REG_MDAY] & 0x3f);
        tmp = regs[DS1307_REG_MONTH] & 0x1f;
        t->tm_mon = bcd2bin(tmp) - 1;
        t->tm_year = bcd2bin(regs[DS1307_REG_YEAR]) + 100;

        if (regs[chip->century_reg] & chip->century_bit &&
            IS_ENABLED(CONFIG_RTC_DRV_DS1307_CENTURY))
                t->tm_year += 100;

        dev_dbg(dev, "%s secs=%d, mins=%d, "
                "hours=%d, mday=%d, mon=%d, year=%d, wday=%d\n",
                "read", t->tm_sec, t->tm_min,
                t->tm_hour, t->tm_mday,
                t->tm_mon, t->tm_year, t->tm_wday);

        return 0;

}

static int ds1307_set_time(struct device *dev, struct rtc_time *t)
{
struct ds1307   *ds1307 = dev_get_drvdata(dev);
        const struct chip_desc *chip = &chips[ds1307->type];
        int             result;
        int             tmp;
        u8              regs[7];

        dev_dbg(dev, "%s secs=%d, mins=%d, "
                "hours=%d, mday=%d, mon=%d, year=%d, wday=%d\n",
                "write", t->tm_sec, t->tm_min,
                t->tm_hour, t->tm_mday,
                t->tm_mon, t->tm_year, t->tm_wday);

        if (t->tm_year < 100)
                return -EINVAL;

#ifdef CONFIG_RTC_DRV_DS1307_CENTURY
        if (t->tm_year > (chip->century_bit ? 299 : 199))
                return -EINVAL;
#else
        if (t->tm_year > 199)
                return -EINVAL;
#endif

        regs[DS1307_REG_SECS] = bin2bcd(t->tm_sec);
        regs[DS1307_REG_MIN] = bin2bcd(t->tm_min);
        regs[DS1307_REG_HOUR] = bin2bcd(t->tm_hour);
                regs[DS1307_REG_WDAY] = bin2bcd(t->tm_wday + 1);
        regs[DS1307_REG_MDAY] = bin2bcd(t->tm_mday);
        regs[DS1307_REG_MONTH] = bin2bcd(t->tm_mon + 1);

        /* assume 20YY not 19YY */
        tmp = t->tm_year - 100;
        regs[DS1307_REG_YEAR] = bin2bcd(tmp);

        if (chip->century_enable_bit)
                regs[chip->century_reg] |= chip->century_enable_bit;
        if (t->tm_year > 199 && chip->century_bit)
                regs[chip->century_reg] |= chip->century_bit;


        dev_dbg(dev, "%s: %7ph\n", "write", regs);

        result = regmap_bulk_write(ds1307->regmap, chip->offset, regs,
                                   sizeof(regs));
        if (result) {
                dev_err(dev, "%s error %d\n", "write", result);
                return result;
        }


        return 0;
}

static const struct i2c_device_id ds1307_id[] = {
	{ "ds1307", ds_1307 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, ds1307_id);

static const struct of_device_id ds1307_of_match[] = {
	{
		.compatible = "dallas,ds1307",
		.data = (void *)ds_1307,
	},
	{ }
};
MODULE_DEVICE_TABLE(of, ds1307_of_match);

static const struct rtc_class_ops ds13xx_rtc_ops = {
	.read_time	= ds1307_get_time,
	.set_time	= ds1307_set_time,
	
};



static const struct regmap_config regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
};
static int ds1307_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct ds1307		*ds1307;
	const void		*match;
	int			err = -ENODEV;
	int			tmp;
	const struct chip_desc	*chip;
	unsigned char		regs[8];
	struct ds1307_platform_data *pdata = dev_get_platdata(&client->dev);
	ds1307 = devm_kzalloc(&client->dev, sizeof(struct ds1307), GFP_KERNEL);
	if (!ds1307)
		return -ENOMEM;

	dev_set_drvdata(&client->dev, ds1307);
	ds1307->dev = &client->dev;
	ds1307->name = client->name;

	ds1307->regmap = devm_regmap_init_i2c(client, &regmap_config);
	if (IS_ERR(ds1307->regmap)) {
		dev_err(ds1307->dev, "regmap allocation failed\n");
		return PTR_ERR(ds1307->regmap);
	}
	i2c_set_clientdata(client, ds1307);
	
	match = device_get_match_data(&client->dev);
	if (match) {
		printk(KERN_DEBUG "Here I am: %s:%i\n", __FUNCTION__, __LINE__);
		ds1307->type = (enum ds_type)match;
		chip = &chips[ds1307->type];
	} else if (id) {
		printk(KERN_DEBUG "Here I am: %s:%i\n", __FUNCTION__, __LINE__);
		chip = &chips[id->driver_data];
		ds1307->type = id->driver_data;
		printk(KERN_DEBUG "Here I am:id->driver_data %s:%i\n", __FUNCTION__, id->driver_data);
	} else {
		printk(KERN_DEBUG "Here I am: %s:%i\n", __FUNCTION__, __LINE__);
		return -ENODEV;
	}
	/* read RTC registers */
	err = regmap_bulk_read(ds1307->regmap, chip->offset, regs,
			       sizeof(regs));
	if (err) {
		dev_dbg(ds1307->dev, "read error %d\n", err);
		goto exit;
	}
	tmp = regs[DS1307_REG_HOUR];
	if(ds1307->type==1)
	{
		if (!(tmp & DS1307_BIT_12HR))
		{
		}

		else{
		/*
		 * Be sure we're in 24 hour mode.  Multi-master systems
		 * take note...
		 */
		tmp = bcd2bin(tmp & 0x1f);
		if (tmp == 12)
			tmp = 0;
		if (regs[DS1307_REG_HOUR] & DS1307_BIT_PM)
			tmp += 12;
		regmap_write(ds1307->regmap, chip->offset + DS1307_REG_HOUR,
			     bin2bcd(tmp));
		}
	}
	ds1307->rtc = devm_rtc_allocate_device(ds1307->dev);
	if (IS_ERR(ds1307->rtc))
		return PTR_ERR(ds1307->rtc);
	ds1307->rtc->ops = chip->rtc_ops ?: &ds13xx_rtc_ops;
	err = devm_rtc_register_device(ds1307->rtc);
	if (err)
		return err;

	return 0;

exit:
	return err;
}


static struct i2c_driver ds1307_driver = {
	.driver = {
		.name	= "rtc-ds1307",
		.of_match_table = ds1307_of_match,
	},
	.probe		= ds1307_probe,
	.id_table	= ds1307_id,
};

module_i2c_driver(ds1307_driver);

MODULE_DESCRIPTION("RTC driver for DS1307 and similar chips");
MODULE_LICENSE("GPL");
