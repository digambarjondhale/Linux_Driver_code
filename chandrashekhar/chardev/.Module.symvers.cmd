cmd_/home/pi/chandrashekhar/chardev/Module.symvers :=  sed 's/ko$$/o/'  /home/pi/chandrashekhar/chardev/modules.order | scripts/mod/modpost -m -a    -o /home/pi/chandrashekhar/chardev/Module.symvers -e -i Module.symvers -T - 