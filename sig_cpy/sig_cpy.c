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

#define SIG_0 0
#define SIG_1 1

void doNothing (int signal);
void handleSignal(int signal);
void cpy(const char* from, const char* to);
void fileCpy(const char* from, const char* to);
char* findLastIteration(const char* str, const char symb);    //if there are no slashes in str - it returns NULL
char* myStrcat(char* dest, const char* src);

struct SWritingByte
{
    char byte;
    size_t bit;
    FILE* output;
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
    if (argv[1][0] != '/')
    {
        printf("%s\n", argv[1]);
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
            printf("%s\n", curr->d_name);
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
    FILE* input = fopen(from, "rt");
    FILE* output = fopen(to, "wt");

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
    pid_t ppid;

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
            sigaddset(&ans, SIG_0);
            sigaddset(&ans, SIG_1);
            getAns.sa_mask = ans;
            sigaction(SIG_0, &getAns, 0);
            sigaction(SIG_1, &getAns, 0);

            while (read(input, &symbIn, 1))
            {
                printf("%c", symbIn);
                for (int i = 128; i > 0; i /= 2)
                {
                    if (symbIn & i)
                    {
                        kill(ppid, SIG_1);
                    }
                    else
                    {
                        kill(ppid, SIG_0);
                    }

                    sigsuspend(&ans);
                }
            }
            exit(0);
        break;
        default:
            memset(&write, 0, sizeof(write));
            write.sa_handler = handleSignal;
            sigset_t signals;
            sigemptyset(&signals);
            sigaddset(&signals, SIG_0);
            sigaddset(&signals, SIG_1);
            write.sa_mask = signals;
            sigaction(SIG_0, &write, 0);
            sigaction(SIG_1, &write, 0);

            waitpid(pid, NULL, 0);  
        break;
    }

    fclose(input);
    fclose(output);
}

void doNothing (int signal)
{
    return;
}

void handleSignal(int signal)
{
    switch (signal)
    {
        case SIG_0:
            writingByte.bit /= 2;
            kill(pid, SIG_0);
        break;
        case SIG_1:
            writingByte.byte += writingByte.bit;
            writingByte.bit /= 2;
            kill(pid, SIG_1);
        break;
        default:
            perror("An error occured while handling signal");
            exit(-1);
        break;
    }
    
    if (writingByte.bit == 0)
    {
        fprintf(writingByte.output,"%c", writingByte.byte);
        writingByte.bit = 128;
    }
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