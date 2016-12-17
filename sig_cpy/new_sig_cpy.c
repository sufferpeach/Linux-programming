#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>

struct SWritingByte
{
    unsigned char byte;
    unsigned char bit;
    int output;
}
typedef SWritingByte;

void fileCpy(const char* from, const char* to);
void doNothing (int signal);
void handleSignal(int signal);
void handleChildDeath(int signal);
char* myStrcat(char* dest, const char* src);
char* findLastIteration(const char* str, const char symb);    //if there are no slashes in str - it returns NULL

SWritingByte writingByte;
pid_t pid;

int main(int argc, char** argv, char** envp)
{
    //checking input
    if (argc != 3)
    {
        perror("Incorrect input of command sig_cpy");
        exit(1);
    }
    if (!strcmp(argv[1], argv[2]))
        return 0;
    //

    //make output directory
    DIR* dir = opendir(argv[2]);
    if (!dir)
    {
        mkdir(argv[2], 0777);
    }
    closedir(dir);
    //

    //construcring path
    char* lastSlashIteration = findLastIteration(argv[1], '/');
    char* pathTo;
    if (lastSlashIteration)
    {
        pathTo = (char*)malloc((strlen(argv[2]) + strlen(argv[1]) + argv[1] - lastSlashIteration + 2) * sizeof(char));
        if (argv[2][strlen(argv[2]) - 1] == '/')
            myStrcat(strcpy(pathTo, argv[2]), lastSlashIteration + 1);
        else
            myStrcat(strcpy(pathTo, argv[2]), lastSlashIteration);
        fileCpy(argv[1], pathTo);
    }
    else
    {
        pathTo = (char*)malloc((strlen(argv[2]) + strlen(argv[1]) + 2) * sizeof(char));
        fileCpy(argv[1], myStrcat(myStrcat(strcpy(pathTo, argv[2]), "/"), argv[1]));
    }
    //

    return 0;
}

void fileCpy(const char* from, const char* to)
{
    int input = open(from, O_RDONLY);
    if (input < 0)
    {
        perror("Error");
        exit(errno);
    }

    struct stat fileStat;
    stat(from, &fileStat);

    int output = open(to, O_WRONLY | O_CREAT, fileStat.st_mode);
    if (output < 0)
    {
        perror("Error");
        exit(errno);
    }

    writingByte.byte = 0;
    writingByte.bit = 128;
    writingByte.output = output;

    char symbIn;
    while (read(input, &symbIn, 1))
            {
                write(output, &symbIn, 1);
            }
    exit(0);
    struct sigaction write;
    struct sigaction getAns;
    struct sigaction childDeath;
    pid_t ppid;

    sigset_t blank;
    sigemptyset(&blank);

    sigset_t signals;
    sigemptyset(&signals);
    sigaddset(&signals, SIGUSR1);
    sigaddset(&signals, SIGUSR2);
    sigprocmask(SIG_BLOCK, &signals, 0);

    switch (pid = fork())
    {
        case -1:
            perror("An error occurred while creating a process");
            exit(errno);
        break;
        case 0:
            ppid = getppid();

            memset(&getAns, 0, sizeof(getAns));
            getAns.sa_handler = doNothing;
            sigset_t ans;
            sigemptyset(&ans);
            sigaddset(&ans, SIGUSR1);
            sigaddset(&ans, SIGUSR2);
            getAns.sa_mask = ans;
            sigaction(SIGUSR1, &getAns, 0);
            sigaction(SIGUSR2, &getAns, 0);

            while (read(input, &symbIn, 1))
            {
                printf("%c\n", symbIn);
                for (char i = 128; i > 0; i /= 2)
                {
                    alarm(1); //cross-platform solution for parent's death
                    if (symbIn & i)                                            
                        kill(ppid, SIGUSR2);
                    else
                        kill(ppid, SIGUSR1);

                    sigsuspend(&blank);
                }
            }
            
            close(input);
            close(output);

            exit(EXIT_SUCCESS);
        break;
        default:
            memset(&childDeath, 0, sizeof(childDeath));
            childDeath.sa_handler = handleChildDeath;
            childDeath.sa_mask = signals;

            memset(&write, 0, sizeof(write));
            write.sa_handler = handleSignal;
            write.sa_mask = signals;
            sigaction(SIGUSR1, &write, 0);
            sigaction(SIGUSR2, &write, 0);
            sigaction(SIGCHLD, &childDeath, 0);

            while(1)
                sigsuspend(&blank);
        break;
    }
}

void doNothing (int signal)
{
    return;
}

void handleSignal(int signal)
{
    switch (signal)
    {
        case SIGUSR1:
            writingByte.bit /= 2;
            kill(pid, signal);
        break;
        case SIGUSR2:
            writingByte.byte += writingByte.bit;
            writingByte.bit /= 2;
            kill(pid, signal);
        break;
    }
    
    if (writingByte.bit == 0)
    {
        write(writingByte.output, &writingByte.byte, 1);
        writingByte.byte = 0;
        writingByte.bit = 128;
    }
}

void handleChildDeath(int signal)
{
    exit(EXIT_SUCCESS);
}

char* findLastIteration(const char* str, const char symb)
{
    for (int i = strlen(str) - 1; i >= 0; i--)
    {
        if (str[i] == '/')
        {
            return (char*)(str + i);
        }
    }

    return NULL;
}

char* myStrcat(char* dest, const char* src)
{
    size_t destLen = strlen(dest);
    int i;

    for (i = 0 ; src[i] != '\0' ; i++)
        dest[destLen + i] = src[i];
    dest[destLen + i] = '\0';

    return dest;
}