cmd_/home/pi/digambar/ldd/custom_dervier/hello_world/Module.symvers :=  sed 's/ko$$/o/'  /home/pi/digambar/ldd/custom_dervier/hello_world/modules.order | scripts/mod/modpost -m -a    -o /home/pi/digambar/ldd/custom_dervier/hello_world/Module.symvers -e -i Module.symvers -T - 
