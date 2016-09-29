#define BUFF_SIZE 4096

#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

typedef struct SBuff
{
    char* data;
    size_t size;
}
SBuff;

SBuff* createBuff(size_t size);
void destroyBuff(SBuff* buff);
void cpy(char* from, char* to);

int main(int argc, char** argv, char** envp)
{
    //if-block
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

    cpy(argv[1], argv[2]);

    return 0;
}

void cpy(char* from, char* to)
{
    DIR* dir;
    dir = opendir(from);
    if (!dir)
    {
        perror("No such file or directory");
        exit(1);
    }
    struct dirent* curr = readdir(dir);
    curr = readdir(dir);
    while ((curr = readdir(dir)) != NULL)
    {
        char* pathFrom = (char*)malloc((strlen(from) + strlen(curr->d_name) + 1) * sizeof(char));
        char* pathTo = (char*)malloc((strlen(from) + strlen(curr->d_name) + 1) * sizeof(char));
        strcat(strcat(strcat(pathFrom, from), "/"), curr->d_name);
        strcat(strcat(strcat(pathTo, to), "/"), curr->d_name);
        struct stat* currStat;
        stat(pathFrom, currStat);
        if (S_ISDIR(currStat->st_mode))
        {
            mkdir(pathTo, currStat->st_mode);
            cpy(pathFrom, pathTo);
            free(pathFrom);
            free(pathTo);
        }
        else
        {
            SBuff* buff = createBuff(BUFF_SIZE);
            FILE* input = fopen(pathFrom, "rt");
            FILE* output = fopen(pathTo, "wt");
            printf("%zu\n", strlen(buff->data));
            while (fgets(buff->data, buff->size, input))
            {
                fprintf(output, "%s", buff->data);
            }
            fclose(input);
            fclose(output);
            destroyBuff(buff);            
        }
    }
}

SBuff* createBuff(size_t size)
{
    SBuff* buff = (SBuff*)malloc(sizeof(SBuff));
    buff->data = (char*)calloc(size + 1, sizeof(char));
    int i = 0;
    printf("%s", buff->data);
    buff->size = size;
    return buff;
}

void destroyBuff(SBuff* buff)
{
    free(buff->data);
    free(buff);
}