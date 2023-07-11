/* DOS CREATE */

#include "user.h"
// #include "kernel/sysproc.c"
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Inapropriate number of arguments\n");
        exit(1);
    }
    int process_priority=atoi(argv[1]);
    int process_pid=atoi(argv[2]);
    set_priority(process_priority,process_pid);
    exit(0);
}