BIN_DIR := ../bin

# Name of the kernel module
MODULE_NAME := i2c_wq

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
	mv i2c_wq.dwo  i2c_wq.ko  i2c_wq.mod  i2c_wq.mod.c  i2c_wq.mod.dwo  i2c_wq.mod.o  i2c_wq.o modules.order  Module.symvers $(BIN_DIR)

# Clean target
clean:
	make -C $(KERNEL_SRC) M=$(BUILD_DIR) clean
	rm -rf $(BIN_DIR)
