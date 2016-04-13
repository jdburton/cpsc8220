#include <stdio.h>
#include <errno.h>
#include <sys/syscall.h>
#include <signal.h>

#define getqvars(wait_time,service_time,num_req) syscall(331,wait_time,service_time,num_req)

int main()
{
    unsigned long wait_time, service_time, num_req;
    getqvars(&wait_time,&service_time,&num_req);

    printf("After %d requests: Total wait time = %d ms; Total service time = %d ms\n",num_req,wait_time,service_time);
    return 0;
}
