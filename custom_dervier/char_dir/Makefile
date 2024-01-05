obj-m += multiple_devic.o

MYDIR=/lib/modules/$(shell uname -r)/build

all:
	make -C $(MYDIR) M=$(shell pwd) modules
clean:
	make -C $(MYDIR) M=$(shell pwd) clean

