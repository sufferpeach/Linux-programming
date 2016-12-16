#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>

void doNothing (int signal);
void handleSignal(int signal);
void handleChildDeath(int signal);
void cpy(const char* from, const char* to);
void fileCpy(const char* from, const char* to);
char* findLastIteration(const char* str, const char symb);    //if there are no slashes in str - it returns NULL
char* myStrcat(char* dest, const char* src);

struct SWritingByte
{
    unsigned char byte;
    unsigned char bit;
    int output;
}
typedef SWritingByte;

SWritingByte writingByte;
pid_t pid;

int main(int argc, char** argv, char** envp)
{
    //check input
    if (argc < 3)
    {
        perror("Incorrect input of function my_cp");
        return 0;
    }
    if (!strcmp(argv[1], argv[2]))
    {
        return 0;
    }
    //

    //make output directory
    DIR* dir = opendir(argv[2]);
    if (!dir)
    {
        mkdir(argv[2], 0777);
    }
    closedir(dir);
    //

    cpy(argv[1], argv[2]);

    return 0;
}

void cpy(const char* from, const char* to)
{
    DIR* dir = opendir(from);
    if (!dir)
    {
        struct stat checkStat;
        if (!stat(from, &checkStat) && S_ISREG(checkStat.st_mode))
        {
            char* lastSlashIteration = findLastIteration(from, '/');
            char* pathTo;
            if (lastSlashIteration)
            {
                pathTo = (char*)malloc((strlen(to) + strlen(from) + from - lastSlashIteration + 2) * sizeof(char));
                fileCpy(from, myStrcat(strcpy(pathTo, to), lastSlashIteration));
            }
            else
            {
                pathTo = (char*)malloc((strlen(to) + strlen(from) + 2) * sizeof(char));
                fileCpy(from, myStrcat(myStrcat(strcpy(pathTo, to), "/"), from));
            }
            free(pathTo);
            return;
        }
        else
        {
            perror("Error");
            exit(1);
        }
    }
    struct dirent* curr;
    while ((curr = readdir(dir)) != NULL)
    {
        if ((strcmp(curr->d_name, ".") && strcmp(curr->d_name, "..")))
        {
            char* pathFrom = (char*)malloc((strlen(from) + strlen(curr->d_name) + 2) * sizeof(char));
            char* pathTo = (char*)malloc((strlen(to) + strlen(curr->d_name) + 2) * sizeof(char));

            //checking for extra characters '/'
            if (from[strlen(from) - 1] == '/')
            {
                myStrcat(strcpy(pathFrom, from), curr->d_name);
            }
            else
            {
                myStrcat(myStrcat(strcpy(pathFrom, from), "/"), curr->d_name);
            }
            if (to[strlen(to) - 1] == '/')
            {
                myStrcat(strcpy(pathTo, to), curr->d_name);
            }
            else
            {
                myStrcat(myStrcat(strcpy(pathTo, to), "/"), curr->d_name);
            }
            //
            
            struct stat currStat;
            stat(pathFrom, &currStat);
            if (S_ISDIR(currStat.st_mode))
            {
                mkdir(pathTo, currStat.st_mode);
                cpy(pathFrom, pathTo);
            }
            else
            {
                fileCpy(pathFrom, pathTo);
            }
            free(pathFrom);
            free(pathTo);
        }
    }
}

void fileCpy(const char* from, const char* to)
{
    int input = open(from, O_RDONLY);
    int output = open(to, O_WRONLY | O_CREAT);

    if(!output)
    {
        perror("An error occurred while creating the file");
        exit(1);
    }

    writingByte.byte = 0;
    writingByte.bit = 128;
    writingByte.output = output;

    char symbIn;
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
            int useless;

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
                for (int i = 128; i > 0; i /= 2)
                {
                    alarm(1); //cross-platform solution for parent's death
                    if (symbIn & i)                                            
                        kill(ppid, SIGUSR2);
                    else
                        kill(ppid, SIGUSR1);

                    sigsuspend(&blank);
                }
            }
            exit(0);
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

    close(input);
    close(output);
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
    exit(0);
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