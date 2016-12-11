#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define TRUE 1

#define CHECK_CMD(cmd) cmd->name && (cmd->arg || cmd->argAmount)
#define MIN(a, b) ((a - b) > 0) ? b : a

typedef struct cmd_t
{
	char* name;
	char** arg;
    size_t argAmount;
}
cmd_t;

typedef struct cmdList_t
{
    cmd_t** cmd;
    size_t cmdAmount;
}
cmdList_t;

char* getLine();                         //memory must be freed afted usage!
cmd_t* constructCmd(char* cmdLine);      //memory must be freed afted usage by following destructor!
void destructCmd(cmd_t* cmd);
cmdList_t* constructCmdList(char* line); //memory must be freed afted usage by following destructor!
void destructCmdList(cmdList_t* cmdList);


int main(int argc, char** argv, char** envp)
{
    //print manual
    printf("Here manual wanna be\n");
    //

    char* line;

    while(TRUE)
    {
        if (!(line = getLine()))
        {
		    perror("Can't get command line from console. Error occured:");
            exit(errno);
	    }
		cmdList_t*  list = constructCmdList(line);
        for (size_t i = 0; i < list->cmdAmount; i++)
        {
            for (size_t j = 0; j < list->cmd[i]->argAmount + 1; j++)
            {
                if (j == 0)
                {
                    printf("%s ", list->cmd[i]->name);
                }
                else
                {
                    printf("%s ", list->cmd[i]->arg[j]);
                }
                //printf("dsaadsdasdasas %zu %zu\n", list->cmdAmount, list->cmd[i]->argAmount);
            }
        }
	}

}

char* getLine()
{	
	char* line = malloc(sizeof(char));
    line[0] = '\0';
    size_t memSize = 1;
    char c, str[2];
    while ((c = getc(stdin)) != '\n')
    {
        memSize++;
        str[0] = c;
        str[1] = '\0';
        line = (char*)realloc(line, (memSize + 1) * sizeof(char));
        strcat(line, str);  
    }

	return line;
}

cmd_t* constructCmd(char* cmdLine)
{
    cmd_t* cmd = (cmd_t*)malloc(sizeof(cmd_t));
    cmd->argAmount = 0;
    char* line = cmdLine;

    if (!line)
    {
        perror("Ivalid command");
        perror(line);
        exit(errno);
    }
    cmd->name = line;
    line = MIN(strchr(line, ' '), strchr(line, '|'));
    while ((*line == '|' || *line == ' ') && line)
    {
        *line = '\0';
        line++;
    }
    printf("%s\n", cmd->name);
    printf("%s\n", line);

    while (line)
    {
        cmd->argAmount++;
        cmd->arg = (char**)realloc(cmd->arg, cmd->argAmount * sizeof(char*));
        cmd->arg[cmd->argAmount - 1] = line;
        line = MIN(strchr(line, ' '), strchr(line, '|'));
        printf("%zu\n", cmd->argAmount);
        while ((*line == '|' || *line == ' ') && line)
        {   
            *line = '\0';
            line++;
        }
    }

    return cmd;
}

void destructCmd(cmd_t* cmd)
{

}

cmdList_t* constructCmdList(char* line)
{
    cmdList_t* cmdList = (cmdList_t*)malloc(sizeof(cmdList));
    cmdList->cmdAmount = 0;
    cmd_t* cmd;
    char* cmdLine = line;

    while (cmdLine)
    {
        printf("dddddd\n");
        cmdList->cmdAmount++;
        cmdList->cmd = (cmd_t**)realloc(cmdList->cmd, cmdList->cmdAmount * sizeof(cmd_t*));
        if (!(CHECK_CMD((cmd = constructCmd(cmdLine)))))
        {
            perror("Ivalid command");
            perror(cmd->name);
            exit(errno);
        }
        cmdList->cmd[cmdList->cmdAmount - 1] = cmd;
        cmdLine = strchr(cmdLine, '|');
    }

    return cmdList;
}

void destructCmdList(cmdList_t* cmdList)
{
    for (int i = 0; i < cmdList->cmdAmount; i++)
    {
        destructCmd(cmdList->cmd[i]);
    }
    free(cmdList->cmd);
    free(cmdList);
}