obj-m:=redmem.o
KDIR:=/usr/src/linux-2.6.32-cut
PWD:=$(shell pwd)
CC:=gcc

modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	
