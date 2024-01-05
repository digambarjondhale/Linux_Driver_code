cmd_/home/pi/utkarsh/my_rtc_driver.mod := printf '%s\n'   my_rtc_driver.o | awk '!x[$$0]++ { print("/home/pi/utkarsh/"$$0) }' > /home/pi/utkarsh/my_rtc_driver.mod
