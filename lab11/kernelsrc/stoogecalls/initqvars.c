#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/blkdev.h>
#include <linux/time.h>
#include <linux/timekeeping.h>

extern struct timespec q_reset_time;

SYSCALL_DEFINE0(initqvars)
{
    q_total_service_time=0;
    q_total_wait_time=0;
    q_total_requests=0;
    getnstimeofday(&q_reset_time);
    return(0);
}
