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
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/clk-provider.h>
#include <linux/regmap.h>
#include <linux/watchdog.h>

/*
 * We can't determine type by probing, but if we expect pre-Linux code
 * to have set the chip up as a clock (turning on the oscillator and
 * setting the date and time), Linux can ignore the non-clock features.
 * That's a natural job for a factory or repair bench.
 */
enum ds_type {
	unknown_ds_type, /* always first and 0 */
	ds_1307,
	//ds_1308,
	//ds_1337,
	//ds_1338,
	//ds_1339,
	//ds_1340,
	//ds_1341,
	//ds_1388,
	//ds_3231,
	//m41t0,
	//m41t00,
	//m41t11,
	//mcp794xx,
	//rx_8025,
	//rx_8130,
	last_ds_type /* always last */
	/* rs5c372 too?  different address... */
};

/* RTC registers don't differ much, except for the century flag */
#define DS1307_REG_SECS		0x00	/* 00-59 */
#define DS1307_BIT_CH		0x80

#define DS1307_REG_MIN		0x01	/* 00-59 */

#define DS1307_REG_HOUR		0x02	/* 00-23, or 1-12{am,pm} */
#	define DS1307_BIT_12HR		0x40	/* in REG_HOUR */
#	define DS1307_BIT_PM		0x20	/* in REG_HOUR */

#define DS1307_REG_WDAY		0x03	/* 01-07 */

#define DS1307_REG_MDAY		0x04	/* 01-31 */
#define DS1307_REG_MONTH	0x05	/* 01-12 */

#define DS1307_REG_YEAR		0x06	/* 00-99 */

/*
 * Other registers (control, status, alarms, trickle charge, NVRAM, etc)
 * start at 7, and they differ a LOT. Only control and status matter for
 * basic RTC date and time functionality; be careful using them.
 */
#define DS1307_REG_CONTROL	0x07		/* or ds1338 */
#	define DS1307_BIT_OUT		0x80
#	define DS1338_BIT_OSF		0x20
#	define DS1307_BIT_SQWE		0x10
#	define DS1307_BIT_RS1		0x02
#	define DS1307_BIT_RS0		0x01


#define DS13XX_TRICKLE_CHARGER_MAGIC	0xa0


#if 0
/* RTC registers don't differ much, except for the century flag */
#define DS1307_REG_SECS		0x00	/* 00-59 */
#	define DS1307_BIT_CH		0x80
#	define DS1340_BIT_nEOSC		0x80
#	define MCP794XX_BIT_ST		0x80
#define DS1307_REG_MIN		0x01	/* 00-59 */
#	define M41T0_BIT_OF		0x80
#define DS1307_REG_HOUR		0x02	/* 00-23, or 1-12{am,pm} */
#	define DS1307_BIT_12HR		0x40	/* in REG_HOUR */
#	define DS1307_BIT_PM		0x20	/* in REG_HOUR */
#	define DS1340_BIT_CENTURY_EN	0x80	/* in REG_HOUR */
#	define DS1340_BIT_CENTURY	0x40	/* in REG_HOUR */
#define DS1307_REG_WDAY		0x03	/* 01-07 */
#	define MCP794XX_BIT_VBATEN	0x08
#define DS1307_REG_MDAY		0x04	/* 01-31 */
#define DS1307_REG_MONTH	0x05	/* 01-12 */
#	define DS1337_BIT_CENTURY	0x80	/* in REG_MONTH */
#define DS1307_REG_YEAR		0x06	/* 00-99 */

/*
 * Other registers (control, status, alarms, trickle charge, NVRAM, etc)
 * start at 7, and they differ a LOT. Only control and status matter for
 * basic RTC date and time functionality; be careful using them.
 */
#define DS1307_REG_CONTROL	0x07		/* or ds1338 */
#	define DS1307_BIT_OUT		0x80
#	define DS1338_BIT_OSF		0x20
#	define DS1307_BIT_SQWE		0x10
#	define DS1307_BIT_RS1		0x02
#	define DS1307_BIT_RS0		0x01
#define DS1337_REG_CONTROL	0x0e
#	define DS1337_BIT_nEOSC		0x80
#	define DS1339_BIT_BBSQI		0x20
#	define DS3231_BIT_BBSQW		0x40 /* same as BBSQI */
#	define DS1337_BIT_RS2		0x10
#	define DS1337_BIT_RS1		0x08
#	define DS1337_BIT_INTCN		0x04
#	define DS1337_BIT_A2IE		0x02
#	define DS1337_BIT_A1IE		0x01
#define DS1340_REG_CONTROL	0x07
#	define DS1340_BIT_OUT		0x80
#	define DS1340_BIT_FT		0x40
#	define DS1340_BIT_CALIB_SIGN	0x20
#	define DS1340_M_CALIBRATION	0x1f
#define DS1340_REG_FLAG		0x09
#	define DS1340_BIT_OSF		0x80
#define DS1337_REG_STATUS	0x0f
#	define DS1337_BIT_OSF		0x80
#	define DS3231_BIT_EN32KHZ	0x08
#	define DS1337_BIT_A2I		0x02
#	define DS1337_BIT_A1I		0x01
#define DS1339_REG_ALARM1_SECS	0x07

#define DS13XX_TRICKLE_CHARGER_MAGIC	0xa0

#define RX8025_REG_CTRL1	0x0e
#	define RX8025_BIT_2412		0x20
#define RX8025_REG_CTRL2	0x0f
#	define RX8025_BIT_PON		0x10
#	define RX8025_BIT_VDET		0x40
#	define RX8025_BIT_XST		0x20

#define RX8130_REG_ALARM_MIN		0x17
#define RX8130_REG_ALARM_HOUR		0x18
#define RX8130_REG_ALARM_WEEK_OR_DAY	0x19
#define RX8130_REG_EXTENSION		0x1c
#define RX8130_REG_EXTENSION_WADA	BIT(3)
#define RX8130_REG_FLAG			0x1d
#define RX8130_REG_FLAG_VLF		BIT(1)
#define RX8130_REG_FLAG_AF		BIT(3)
#define RX8130_REG_CONTROL0		0x1e
#define RX8130_REG_CONTROL0_AIE		BIT(3)
#define RX8130_REG_CONTROL1		0x1f
#define RX8130_REG_CONTROL1_INIEN	BIT(4)
#define RX8130_REG_CONTROL1_CHGEN	BIT(5)

#define MCP794XX_REG_CONTROL		0x07
#	define MCP794XX_BIT_ALM0_EN	0x10
#	define MCP794XX_BIT_ALM1_EN	0x20
#define MCP794XX_REG_ALARM0_BASE	0x0a
#define MCP794XX_REG_ALARM0_CTRL	0x0d
#define MCP794XX_REG_ALARM1_BASE	0x11
#define MCP794XX_REG_ALARM1_CTRL	0x14
#	define MCP794XX_BIT_ALMX_IF	BIT(3)
#	define MCP794XX_BIT_ALMX_C0	BIT(4)
#	define MCP794XX_BIT_ALMX_C1	BIT(5)
#	define MCP794XX_BIT_ALMX_C2	BIT(6)
#	define MCP794XX_BIT_ALMX_POL	BIT(7)
#	define MCP794XX_MSK_ALMX_MATCH	(MCP794XX_BIT_ALMX_C0 | \
					 MCP794XX_BIT_ALMX_C1 | \
					 MCP794XX_BIT_ALMX_C2)

#define M41TXX_REG_CONTROL	0x07
#	define M41TXX_BIT_OUT		BIT(7)
#	define M41TXX_BIT_FT		BIT(6)
#	define M41TXX_BIT_CALIB_SIGN	BIT(5)
#	define M41TXX_M_CALIBRATION	GENMASK(4, 0)

#define DS1388_REG_WDOG_HUN_SECS	0x08
#define DS1388_REG_WDOG_SECS		0x09
#define DS1388_REG_FLAG			0x0b
#	define DS1388_BIT_WF		BIT(6)
#	define DS1388_BIT_OSF		BIT(7)
#define DS1388_REG_CONTROL		0x0c
#	define DS1388_BIT_RST		BIT(0)
#	define DS1388_BIT_WDE		BIT(1)
#	define DS1388_BIT_nEOSC		BIT(7)

/* negative offset step is -2.034ppm */
#define M41TXX_NEG_OFFSET_STEP_PPB	2034
/* positive offset step is +4.068ppm */
#define M41TXX_POS_OFFSET_STEP_PPB	4068
/* Min and max values supported with 'offset' interface by M41TXX */
#define M41TXX_MIN_OFFSET	((-31) * M41TXX_NEG_OFFSET_STEP_PPB)
#define M41TXX_MAX_OFFSET	((31) * M41TXX_POS_OFFSET_STEP_PPB)

#endif

struct ds1307 {
	enum ds_type		type;
	struct device		*dev;
	struct regmap		*regmap;
	const char		*name;
	struct rtc_device	*rtc;
#ifdef CONFIG_COMMON_CLK
	struct clk_hw		clks[2];
#endif
};

struct chip_desc {
	unsigned		alarm:1;
	u16			nvram_offset;
	u16			nvram_size;
	u8			offset; /* register's offset */
	u8			century_reg;
	u8			century_enable_bit;
	u8			century_bit;
	u8			bbsqi_bit;
	irq_handler_t		irq_handler;
	const struct rtc_class_ops *rtc_ops;
	u16			trickle_charger_reg;
	u8			(*do_trickle_setup)(struct ds1307 *, u32,
						    bool);
	/* Does the RTC require trickle-resistor-ohms to select the value of
	 * the resistor between Vcc and Vbackup?
	 */
	bool			requires_trickle_resistor;
	/* Some RTC's batteries and supercaps were charged by default, others
	 * allow charging but were not configured previously to do so.
	 * Remember this behavior to stay backwards compatible.
	 */
	bool			charge_default;
};

static const struct chip_desc chips[last_ds_type];

static int ds1307_get_time(struct device *dev, struct rtc_time *t)
{
	struct ds1307	*ds1307 = dev_get_drvdata(dev);
	int		tmp, ret;
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
	struct ds1307	*ds1307 = dev_get_drvdata(dev);
	const struct chip_desc *chip = &chips[ds1307->type];
	int		result;
	int		tmp;
	u8		regs[7];

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


static const struct chip_desc chips[last_ds_type] = {
	[ds_1307] = {
		.nvram_offset	= 8,
		.nvram_size	= 56,
	},
	
};

static const struct i2c_device_id ds1307_id[] = {
	{ "ds1307", ds_1307 },
	
	{ }
};
MODULE_DEVICE_TABLE(i2c, ds1307_id);

static const struct of_device_id ds1307_of_match[] = {
	{
		.compatible = "dallas,ds1307",
		.data = (void *)ds_1307
	},
	
	//{
	//	.compatible = "pericom,pt7c4338",
	//	.data = (void *)ds_1307
	//},
	
	{ }
};
MODULE_DEVICE_TABLE(of, ds1307_of_match);


/*----------------------------------------------------------------------*/

static const struct rtc_class_ops ds13xx_rtc_ops = {
	.read_time	= ds1307_get_time,
	.set_time	= ds1307_set_time,
	
};



/*----------------------------------------------------------------------*/


/*----------------------------------------------------------------------*/

static u8 ds1307_trickle_init(struct ds1307 *ds1307,
			      const struct chip_desc *chip)
{
	u32 ohms, chargeable;
	bool diode = chip->charge_default;

	if (!chip->do_trickle_setup)
		return 0;

	if (device_property_read_u32(ds1307->dev, "trickle-resistor-ohms",
				     &ohms) && chip->requires_trickle_resistor)
		return 0;

	/* aux-voltage-chargeable takes precedence over the deprecated
	 * trickle-diode-disable
	 */
	if (!device_property_read_u32(ds1307->dev, "aux-voltage-chargeable",
				     &chargeable)) {
		switch (chargeable) {
		case 0:
			diode = false;
			break;
		case 1:
			diode = true;
			break;
		default:
			dev_warn(ds1307->dev,
				 "unsupported aux-voltage-chargeable value\n");
			break;
		}
	} else if (device_property_read_bool(ds1307->dev,
					     "trickle-diode-disable")) {
		diode = false;
	}

	return chip->do_trickle_setup(ds1307, ohms, diode);
}



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
	bool			want_irq;
	bool			ds1307_can_wakeup_device = false;
	unsigned char		regs[8];
	struct ds1307_platform_data *pdata = dev_get_platdata(&client->dev);
	u8			trickle_charger_setup = 0;

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
		ds1307->type = (enum ds_type)match;
		chip = &chips[ds1307->type];
	} else if (id) {
		chip = &chips[id->driver_data];
		ds1307->type = id->driver_data;
	} else {
		return -ENODEV;
	}

	want_irq = client->irq > 0 && chip->alarm;

	if (!pdata)
		trickle_charger_setup = ds1307_trickle_init(ds1307, chip);
	else if (pdata->trickle_charger_setup)
		trickle_charger_setup = pdata->trickle_charger_setup;

	if (trickle_charger_setup && chip->trickle_charger_reg) {
		dev_dbg(ds1307->dev,
			"writing trickle charger info 0x%x to 0x%x\n",
			trickle_charger_setup, chip->trickle_charger_reg);
		regmap_write(ds1307->regmap, chip->trickle_charger_reg,
			     trickle_charger_setup);
	}
#if 0
/*
 * For devices with no IRQ directly connected to the SoC, the RTC chip
 * can be forced as a wakeup source by stating that explicitly in
 * the device's .dts file using the "wakeup-source" boolean property.
 * If the "wakeup-source" property is set, don't request an IRQ.
 * This will guarantee the 'wakealarm' sysfs entry is available on the device,
 * if supported by the RTC.
 */
	if (chip->alarm && device_property_read_bool(&client->dev, "wakeup-source"))
		ds1307_can_wakeup_device = true;

	switch (ds1307->type) {
	case ds_1337:
	case ds_1339:
	case ds_1341:
	case ds_3231:
		/* get registers that the "rtc" read below won't read... */
		err = regmap_bulk_read(ds1307->regmap, DS1337_REG_CONTROL,
				       regs, 2);
		if (err) {
			dev_dbg(ds1307->dev, "read error %d\n", err);
			goto exit;
		}

		/* oscillator off?  turn it on, so clock can tick. */
		if (regs[0] & DS1337_BIT_nEOSC)
			regs[0] &= ~DS1337_BIT_nEOSC;

		/*
		 * Using IRQ or defined as wakeup-source?
		 * Disable the square wave and both alarms.
		 * For some variants, be sure alarms can trigger when we're
		 * running on Vbackup (BBSQI/BBSQW)
		 */
		if (want_irq || ds1307_can_wakeup_device) {
			regs[0] |= DS1337_BIT_INTCN | chip->bbsqi_bit;
			regs[0] &= ~(DS1337_BIT_A2IE | DS1337_BIT_A1IE);
		}

		regmap_write(ds1307->regmap, DS1337_REG_CONTROL,
			     regs[0]);

		/* oscillator fault?  clear flag, and warn */
		if (regs[1] & DS1337_BIT_OSF) {
			regmap_write(ds1307->regmap, DS1337_REG_STATUS,
				     regs[1] & ~DS1337_BIT_OSF);
			dev_warn(ds1307->dev, "SET TIME!\n");
		}
		break;

	case rx_8025:
		err = regmap_bulk_read(ds1307->regmap,
				       RX8025_REG_CTRL1 << 4 | 0x08, regs, 2);
		if (err) {
			dev_dbg(ds1307->dev, "read error %d\n", err);
			goto exit;
		}

		/* oscillator off?  turn it on, so clock can tick. */
		if (!(regs[1] & RX8025_BIT_XST)) {
			regs[1] |= RX8025_BIT_XST;
			regmap_write(ds1307->regmap,
				     RX8025_REG_CTRL2 << 4 | 0x08,
				     regs[1]);
			dev_warn(ds1307->dev,
				 "oscillator stop detected - SET TIME!\n");
		}

		if (regs[1] & RX8025_BIT_PON) {
			regs[1] &= ~RX8025_BIT_PON;
			regmap_write(ds1307->regmap,
				     RX8025_REG_CTRL2 << 4 | 0x08,
				     regs[1]);
			dev_warn(ds1307->dev, "power-on detected\n");
		}

		if (regs[1] & RX8025_BIT_VDET) {
			regs[1] &= ~RX8025_BIT_VDET;
			regmap_write(ds1307->regmap,
				     RX8025_REG_CTRL2 << 4 | 0x08,
				     regs[1]);
			dev_warn(ds1307->dev, "voltage drop detected\n");
		}

		/* make sure we are running in 24hour mode */
		if (!(regs[0] & RX8025_BIT_2412)) {
			u8 hour;

			/* switch to 24 hour mode */
			regmap_write(ds1307->regmap,
				     RX8025_REG_CTRL1 << 4 | 0x08,
				     regs[0] | RX8025_BIT_2412);

			err = regmap_bulk_read(ds1307->regmap,
					       RX8025_REG_CTRL1 << 4 | 0x08,
					       regs, 2);
			if (err) {
				dev_dbg(ds1307->dev, "read error %d\n", err);
				goto exit;
			}

			/* correct hour */
			hour = bcd2bin(regs[DS1307_REG_HOUR]);
			if (hour == 12)
				hour = 0;
			if (regs[DS1307_REG_HOUR] & DS1307_BIT_PM)
				hour += 12;

			regmap_write(ds1307->regmap,
				     DS1307_REG_HOUR << 4 | 0x08, hour);
		}
		break;
	case ds_1388:
		err = regmap_read(ds1307->regmap, DS1388_REG_CONTROL, &tmp);
		if (err) {
			dev_dbg(ds1307->dev, "read error %d\n", err);
			goto exit;
		}

		/* oscillator off?  turn it on, so clock can tick. */
		if (tmp & DS1388_BIT_nEOSC) {
			tmp &= ~DS1388_BIT_nEOSC;
			regmap_write(ds1307->regmap, DS1388_REG_CONTROL, tmp);
		}
		break;
	default:
		break;
	}
#endif
	/* read RTC registers */
	err = regmap_bulk_read(ds1307->regmap, chip->offset, regs,
			       sizeof(regs));
	if (err) {
		dev_dbg(ds1307->dev, "read error %d\n", err);
		goto exit;
	}
#if 0
	if (ds1307->type == mcp794xx &&
	    !(regs[DS1307_REG_WDAY] & MCP794XX_BIT_VBATEN)) {
		regmap_write(ds1307->regmap, DS1307_REG_WDAY,
			     regs[DS1307_REG_WDAY] |
			     MCP794XX_BIT_VBATEN);
	}
#endif
	tmp = regs[DS1307_REG_HOUR];
	switch(ds1307->type) {
	case ds_1307:
		if (!(tmp & DS1307_BIT_12HR))
			break;

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
		break;
	default:
		break;
	}
#if 0	
	switch (ds1307->type) {
	case ds_1340:
	case m41t0:
	case m41t00:
	case m41t11:
		/*
		 * NOTE: ignores century bits; fix before deploying
		 * systems that will run through year 2100.
		 */
		break;
	case rx_8025:
		break;
	default:
		if (!(tmp & DS1307_BIT_12HR))
			break;

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

#endif	
	ds1307->rtc = devm_rtc_allocate_device(ds1307->dev);
	if (IS_ERR(ds1307->rtc))
		return PTR_ERR(ds1307->rtc);

	if (want_irq || ds1307_can_wakeup_device)
		device_set_wakeup_capable(ds1307->dev, true);
	else
		clear_bit(RTC_FEATURE_ALARM, ds1307->rtc->features);

	if (ds1307_can_wakeup_device && !want_irq) {
		dev_info(ds1307->dev,
			 "'wakeup-source' is set, request for an IRQ is disabled!\n");
		/* We cannot support UIE mode if we do not have an IRQ line */
		clear_bit(RTC_FEATURE_UPDATE_INTERRUPT, ds1307->rtc->features);
	}


	ds1307->rtc->ops = chip->rtc_ops ?: &ds13xx_rtc_ops;
	//err = ds1307_add_frequency_test(ds1307);
	//if (err)
	//	return err;

	err = devm_rtc_register_device(ds1307->rtc);
	if (err)
		return err;


	//ds1307_hwmon_register(ds1307);
	//ds1307_clks_register(ds1307);
	//ds1307_wdt_register(ds1307);

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
