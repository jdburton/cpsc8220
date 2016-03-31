#include <linux/linkage.h>
#include <linux/kernel.h>                                                       
#include <linux/syscalls.h>                                                     
#include <linux/sched.h>                                                     
                                                                                
SYSCALL_DEFINE2(smunch, pid_t, pid, unsigned long, bit_pattern)
{
    
    int ret = 0;
    struct pid * pid_s;
    struct task_struct * t;
    unsigned long flags;

    if ( pid > 0)
    {
    // get the pid from the pid_t
        rcu_read_lock();
        pid_s = find_vpid(pid);
        rcu_read_unlock();

    // get the task_struct from the pid
        rcu_read_lock();
        t = pid_task(pid_s,PIDTYPE_PID);
        rcu_read_unlock();

    lock_task_sighand(t,&flags);
    // set the task_state to TASK_INTERRUPTABLE
    set_task_state(t,TASK_INTERRUPTIBLE);
    
    //set the signal_pending bits.
    t->pending.signal.sig[0] = bit_pattern;
    // wake up bomb
    signal_wake_up(t,1);
    unlock_task_sighand(t,&flags);
    }
    
    return ret;
}




