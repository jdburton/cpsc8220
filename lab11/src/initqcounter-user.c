#include <stdio.h>
#include <errno.h>
#include <sys/syscall.h>
#include <signal.h>

#define initqcounter() syscall(330)

int main()
{
    initqcounter();
    return 0;
}
