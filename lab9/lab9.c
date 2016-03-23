#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void handler(void)
{
        printf("handler called\n");
}

int signal_child(void)
{
        int rc = 0;

        return rc;
}

int main(int argc, char **argv)
{
        int pid, rc = 0;

        switch (pid = fork()) {
                case 0:  // Child
                        signal(SIGUSR1, handler);

                        while (1) {
                                printf("child running\n");
                                sleep(1);
                        }

                        break;

                default:  // Parent
                        while (1) {
                                printf("Parent sleeping\n");
                                sleep(10);
                                printf("Parent waking, sending SIGUSR1 to child\n");
                                rc = kill(pid, SIGUSR1);
                                printf("kill returned %d\n", rc);
                        }
        }

        return rc;
}
