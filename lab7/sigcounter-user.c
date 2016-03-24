#include <stdio.h>
#include <errno.h>
#include <sys/syscall.h>
#include <signal.h>

#define init_sigcounter(pid) syscall(326,pid)
#define get_sigcounter(signumber) syscall(327,signumber)

void happy()
{
  int ret,stop;
  printf("child is happy\n");
  ret = get_sigcounter(SIGUSR1);
  stop = get_sigcounter(SIGUSR2);
  printf("SIGUSR1 has been sent to child %d times\n",ret);
  printf("SIGUSR2 has been sent to child %d times\n",stop);
  
}


main()
{
	int pid, ret;
	
	switch(pid = fork())
	{
		case 0: // child
		        while(1)
			{
			signal(SIGUSR1,happy);  //redirect SIGUSR1 to happy
			signal(SIGUSR2,happy);
			printf("child is playing\n");
			sleep(1);
			}
			break;
		default:
			init_sigcounter(pid);
			while(1)
			{
				printf("parent is going to sleep\n");
				sleep(1);
				printf("parent wakes up ... checks on child");
				ret = kill(pid,SIGUSR1);
				sleep(2);
				printf("kill returned %d\n"); //is this right?
				ret = kill(pid,SIGUSR2);
				
			}
	}
}