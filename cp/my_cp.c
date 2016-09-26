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
void cpy(char* from, char* to, SBuff* buff);

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

    SBuff* buff = createBuff(BUFF_SIZE);
    cpy(argv[1], argv[2], buff);
    return 0;
}

void cpy(char* from, char* to, SBuff* buff)
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
        printf("%s\n", pathFrom);
        if (S_ISDIR(currStat->st_mode))
        {
            mkdir(pathTo, currStat->st_mode);
            cpy(pathFrom, pathTo, buff);
            free(pathFrom);
            free(pathTo);
        }
        else
        {
            FILE* input = fopen(pathFrom, "rt");
            FILE* output = fopen(pathTo, "wt");
            printf("asd\n");
            while (fgets(buff->data, buff->size, input))
            {
                fprintf(output, "%s", buff->data);
            }
            fclose(input);
            fclose(output);
        }
    }
}

SBuff* createBuff(size_t size)
{
    SBuff* buff = malloc(sizeof(SBuff));
    buff->data = malloc(size);
    buff->size = size;
    return buff;
}