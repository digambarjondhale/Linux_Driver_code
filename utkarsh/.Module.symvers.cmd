cmd_/home/pi/utkarsh/Module.symvers :=  sed 's/ko$$/o/'  /home/pi/utkarsh/modules.order | scripts/mod/modpost -m -a    -o /home/pi/utkarsh/Module.symvers -e -i Module.symvers -T - 
