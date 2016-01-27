#
# Makefile for kyouko3 driver
#

obj-m += kyouko3.o

default: 
	$(MAKE) -C /usr/src/linux M=$(PWD) modules
		
clean:
	rm *.ko
	rm *.o
	rm *.mod.c 