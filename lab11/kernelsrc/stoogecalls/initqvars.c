#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/blkdev.h>

extern unsigned long q_total_service_time;                                                                                                                                       
extern unsigned long q_total_wait_time;
extern unsigned long q_total_requests;
extern unsigned long q_bad_requests;


SYSCALL_DEFINE0(initqvars)
{
    q_total_service_time=0;
    q_total_wait_time=0;
    q_total_requests=0;
    q_bad_requests=0;
    return(0);
}
