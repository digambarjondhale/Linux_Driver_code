obj-m += pltfrom_device_setup.o

MYDIR=/lib/modules/$(shell uname -r)/build

all:
	make -C $(MYDIR) M=$(shell pwd) modules
clean:
	make -C $(MYDIR) M=$(shell pwd) clean

