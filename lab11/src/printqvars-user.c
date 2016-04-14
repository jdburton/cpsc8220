#include <stdio.h>
#include <errno.h>
#include <sys/syscall.h>
#include <signal.h>

#define getqvars(wait_time,service_time,num_req,num_bad) syscall(331,wait_time,service_time,num_req,num_bad)

int main()
{
    unsigned long wait_time, service_time, num_req,num_bad;
    getqvars(&wait_time,&service_time,&num_req,&num_bad);

    printf("After %u requests (excluding %u bad): Total wait time = %u ms; Total service time = %u ms\n",num_req,num_bad, wait_time,service_time);
    return 0;
}
