/*
 ============================================================================
 Name        : kyouko3-test2.c
 Author      : James Burton
 Version     :
 Copyright   : Copyright 2016
 Description : Hello World as a Linux kernel driver
 ============================================================================
 */
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <assert.h>
#include "../kernelsrc/kyouko3-registers.h"




struct u_kyouko3_dev
{
	unsigned int * u_control_base;
	unsigned int * u_framebuffer;

} kyouko3;


unsigned int U_READ_REG(unsigned int reg)
{


	return(*(kyouko3.u_control_base + ( reg >> 2 )));

}

int main()
{

	int fd;
	int result;

	fd = open("/dev/kyouko3",O_RDWR);
	if ( fd == -1)
	{
		printf("Cannot open /dev/kyouko3. Sorry.\n");
		return fd;
	}
	// Memory map device control region.
	kyouko3.u_control_base=mmap(0,KYOUKO3_CONTROL_SIZE,PROT_WRITE|PROT_READ,MAP_SHARED,fd,MMAP_CONTROL);
	result = U_READ_REG(Device_RAM);
	printf("RAM size in MB is: %d\n", result);
	assert(result == 32);

	// Memory map framebuffer
	kyouko3.u_framebuffer=mmap(0,result * 1024 * 1024,PROT_WRITE|PROT_READ,MAP_SHARED,fd, MMAP_FRAMEBUFFER);



	ioctl(fd,VMODE,GRAPHICS_ON);
/*
	for ( i = 200; i < 201 * 1024; i++)
	{
		U_WRITE_FB(i,0xff0000);
	}

	ioctl(FIFO_FLUSH,)*/
	close(fd);
	return 0;


}
