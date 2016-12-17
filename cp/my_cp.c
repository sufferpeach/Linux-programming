#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

#define BUFF_SIZE 4096

typedef struct SBuff
{
    char* data;
    size_t size;
}
SBuff;

void cpy(const char* from, const char* to);
void fileCpy(const char* from, const char* to);
SBuff* createBuff(const size_t size);                         //memory must be freed by following destructor
void destroyBuff(SBuff* buff);
char* findLastIteration(const char* str, const char symb);    //if there are no slashes in str - it returns NULL
char* myStrcat(char* dest, const char* src);

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
    SBuff* buff = createBuff(BUFF_SIZE);
    FILE* input = fopen(from, "rt");
    FILE* output = fopen(to, "wt");
    while (fgets(buff->data, buff->size, input))
    {
        fprintf(output, "%s", buff->data);
    }
    fclose(input);
    fclose(output);
    destroyBuff(buff);
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

SBuff* createBuff(size_t size)
{
    SBuff* buff = (SBuff*)malloc(sizeof(SBuff));
    buff->data = (char*)malloc(size * sizeof(char));
    buff->size = size;
    return buff;
}

void destroyBuff(SBuff* buff)
{
    free(buff->data);
    free(buff);
}