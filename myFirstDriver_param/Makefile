obj-m += Driver_with_param.o
KDIR = /usr/src/linux-headers-4.4.0-87-generic
all:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	rm -rf *.o *.ko *.mod.* *.symvers *.order
