cmd_/home/pi/chandrashekhar/chardev/mychrdev.mod := printf '%s\n'   mychrdev.o | awk '!x[$$0]++ { print("/home/pi/chandrashekhar/chardev/"$$0) }' > /home/pi/chandrashekhar/chardev/mychrdev.mod
