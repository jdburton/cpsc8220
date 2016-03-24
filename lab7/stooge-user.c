#include <stdio.h>
#include <errno.h>
#include <sys/syscall.h>

#define the_goob(arg) syscall(325,arg)

int main()
{
int ret;
ret = the_goob(5);
fprintf(stderr,"syscall returned %d\n",ret);
return 0;

}
