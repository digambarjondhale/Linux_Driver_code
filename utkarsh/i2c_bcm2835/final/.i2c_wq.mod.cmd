cmd_/home/pi/utkarsh/i2c_bcm2835/final/i2c_wq.mod := printf '%s\n'   i2c_wq.o | awk '!x[$$0]++ { print("/home/pi/utkarsh/i2c_bcm2835/final/"$$0) }' > /home/pi/utkarsh/i2c_bcm2835/final/i2c_wq.mod
