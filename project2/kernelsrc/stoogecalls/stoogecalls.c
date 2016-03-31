#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>

SYSCALL_DEFINE1(goober,int,myarg)
{
printk(KERN_ALERT "Hello from %d\n", myarg);
return(1);
}
