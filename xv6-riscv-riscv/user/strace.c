/* DOS CREATE */

#include "user.h"

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Less number of arguments provided\n");
        exit(1);
    }
    int pid = fork();
    if (pid == 0)
    {
        trace(atoi(argv[1]));
        exec(argv[2], argv + 2);
        exit(0);
    }
    wait(0);
    exit(0);
}