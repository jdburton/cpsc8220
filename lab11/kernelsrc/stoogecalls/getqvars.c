#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/blkdev.h>

SYSCALL_DEFINE3(getqvars, int *, service_time, int *, wait_time, int *, num_requests)
{
    *service_time = q_total_service_time;
    *wait_time = q_total_wait_time;
    *num_requests = q_total_requests;
    return(0);
}
