#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>

SYSCALL_DEFINE1(get_sigcounter,int,signumber)
{
    struct siginfo info;
    struct task_struct * p;
    unsigned long flags = 0;
    unsigned int count = 0;
   
    struct sighand_struct * sighand;


    info.si_signo = signumber;
    info.si_errno = 0;
    info.si_code = SI_USER;
    info.si_pid = task_tgid_vnr(current);
    info.si_uid = from_kuid_munged(current_user_ns(), current_uid());

    p = pid_task(find_vpid(info.si_pid),PIDTYPE_PID);

    sighand = lock_task_sighand(p,&flags);

    count = sighand->sigcounter[signumber];
    unlock_task_sighand(p,&flags);

    return count;  
  
}


