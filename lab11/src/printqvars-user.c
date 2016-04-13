#include <stdio.h>
#include <errno.h>
#include <sys/syscall.h>
#include <signal.h>

#define getqvars(wait_time,service_time,num_req) syscall(331,wait_time,service_time,num_req)

int main()
{
    unsigned long wait_time, service_time, num_req;
    getqvars(&wait_time,&service_time,&num_req);

    printf("After %u requests: Total wait time = %u ms; Total service time = %u ms\n",num_req,wait_time,service_time);
    return 0;
}
