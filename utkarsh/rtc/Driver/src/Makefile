BIN_DIR := ../bin

# Name of the kernel module
MODULE_NAME := my_rtc_driver

# Kernel source directory
KERNEL_SRC := /lib/modules/$(shell uname -r)/build

# Build directory
BUILD_DIR := $(shell pwd)

# Build target
obj-m += $(MODULE_NAME).o

# Default target
all: $(BIN_DIR)

$(BIN_DIR): $(KERNEL_SRC)
	mkdir -p $(BIN_DIR)
	make -C $(KERNEL_SRC) M=$(BUILD_DIR) modules
	mv my_rtc_driver.dwo my_rtc_driver.ko my_rtc_driver.mod my_rtc_driver.mod.c my_rtc_driver.mod.dwo my_rtc_driver.mod.o my_rtc_driver.o modules.order  Module.symvers $(BIN_DIR)

# Clean target
clean:
	make -C $(KERNEL_SRC) M=$(BUILD_DIR) clean
	rm -rf $(BIN_DIR)

