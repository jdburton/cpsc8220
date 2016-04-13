#include <stdio.h>
#include <errno.h>
#include <sys/syscall.h>
#include <signal.h>

#define initqvars() syscall(330)
#define getqvars(wait_time,service_time,num_req) syscall(331,wait_time,service_time,num_req)

int main()
{

    unsigned long wait_time, service_time, num_req;
    initqvars();
    getqvars(&wait_time,&service_time,&num_req);

    printf("After %u requests: Total wait time = %u; Total service time = %u\n",num_req,wait_time,service_time);
    
    return 0;
}