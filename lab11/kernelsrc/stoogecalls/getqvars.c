#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/blkdev.h>

extern unsigned long q_total_service_time;                                                                                                                                       
extern unsigned long q_total_wait_time;
extern unsigned long q_total_requests;
extern unsigned long q_bad_requests;


SYSCALL_DEFINE4(getqvars, unsigned long *, service_time, unsigned long *, wait_time, unsigned long *, num_requests, unsigned long *, num_bad)
{
    *service_time = q_total_service_time;
    *wait_time = q_total_wait_time;
    *num_requests = q_total_requests;
    *num_bad = q_bad_requests;
    return(0);
}
