cmd_/home/pi/chandrashekhar/i2c_bcm2835/Module.symvers :=  sed 's/ko$$/o/'  /home/pi/chandrashekhar/i2c_bcm2835/modules.order | scripts/mod/modpost -m -a    -o /home/pi/chandrashekhar/i2c_bcm2835/Module.symvers -e -i Module.symvers -T - 