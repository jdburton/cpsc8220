#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>

SYSCALL_DEFINE1(init_sigcounter,int,pid)
{
    struct task_struct * p;
    struct sighand_struct * sighand;
    int i=0;
    unsigned long flags = 0;

    p = pid_task(find_vpid(pid),PIDTYPE_PID);

    sighand = lock_task_sighand(p,&flags);

    for ( i = 0; i < _NSIG; i++)
    {
        sighand->sigcounter[i] = 0;
    }

    unlock_task_sighand(p,&flags);
    return (1);  
  
}


