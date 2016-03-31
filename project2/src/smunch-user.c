#include <stdio.h>
#include <errno.h>
#include <sys/syscall.h>
#include <stdlib.h>

#define smunch(pid,bit_pattern) syscall(329,pid,bit_pattern)

int main(int argc, char * argv[])
{
    int pid = 0;
    unsigned long bit_pattern = 0;
    int ret;
    
    if (argc < 3)
    {
        fprintf(stderr,"smunge pid bit_pattern\n");
        return -1;
    }
    pid = atoi(argv[1]);
    bit_pattern = strtoul(argv[2],NULL,16);

    ret =  smunch(pid,bit_pattern);

    fprintf(stderr,"syscall returned %d\n",ret);
    return ret;

}
