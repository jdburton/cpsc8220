#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/delay.h>

#define SMUNCH_KILL_BIT ( 1UL << (SIGKILL - 1))

SYSCALL_DEFINE2(smunch, pid_t, pid, unsigned long, bit_pattern)
{

	int ret = -1;		// assume failure
	struct pid *pid_s;
	struct task_struct *t;
	unsigned long flags;

	if (pid <= 0) {
		return -1;
	}
	// get the pid from the pid_t
	rcu_read_lock();
	pid_s = find_vpid(pid);
	rcu_read_unlock();
	if (!pid_s)
		goto exit;

	// get the task_struct from the pid
	rcu_read_lock();
	t = pid_task(pid_s, PIDTYPE_PID);
	rcu_read_unlock();
	if (!t)
		goto exit;

	if (bit_pattern & SMUNCH_KILL_BIT) {
		bit_pattern = SMUNCH_KILL_BIT;	// only send kill if it is only bit set.
	}
	// hunt for zombies.
	if (t->exit_state & EXIT_ZOMBIE) {
		printk(KERN_ALERT "pid %d is a zombie!\n", pid);
		// only clean up on a kill
		if (bit_pattern == SMUNCH_KILL_BIT) {
			// cleanup after the zombie
			printk(KERN_ALERT
			       "Killing zombie %d with your wooden sword!\n",
			       pid);

			msleep(20);
			release_task(t);
		}
		ret = 0;	// success!
		goto exit;

	}
	printk(KERN_ALERT "pid %d is not a zombie. Whew!\n", pid);

	lock_task_sighand(t, &flags);

	// set the task_state to TASK_INTERRUPTABLE
	set_task_state(t, TASK_INTERRUPTIBLE);

	//set the signal_pending bits.
	t->pending.signal.sig[0] = bit_pattern;

	// the wake up bomb
	signal_wake_up(t, 1);

	unlock_task_sighand(t, &flags);
	ret = 0;		// we succeeded! 

 exit:
	return ret;
}
