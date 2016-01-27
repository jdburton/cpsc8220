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

#define KYOUKO3_CONTROL_SIZE 65536
#define DeviceRAM 0x0020

struct u_kyouko3_dev
{
	unsigned int * u_control_base;

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
	kyouko3.u_control_base=mmap(0,KYOUKO3_CONTROL_SIZE,PROT_WRITE|PROT_READ,MAP_SHARED,fd,0);
	result = U_READ_REG(DeviceRAM);
	printf("RAM size in MB is: %d\n", result);
	close(fd);
	return 0;


}
