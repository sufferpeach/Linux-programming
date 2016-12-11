#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define CHECK_CMD(cmd) name && (arg || argAmount)

typedef struct cmd_t
{
	char* name;
	char** arg;
    size_t argAmount;
}
cmd_t;

typedef struct cmdList_t
{
    cmd_t* cmd;
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
        for (int i = 0; i < list->cmdAmount; i++)
        {
            for (int j = 0; j < list->cmd[i]->argAmount + 1; j++)
            {
                if (j == 0)
                {
                    printf("%s", list->cmd[i]->name);
                }
                else
                {
                    printf("%s", list->cmd[i]->arg[j]);
                }
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
    cmdLine = strtok(line," |");

    if (!cmdLine)
    {
        perror("Ivalid command");
        perror(cmdLine);
        exit(errno);
    }
    cmd->name = cmdLine;

    while (cmdLine != NULL)
    {
        argAmount++;
        cmd->arg = (cmd_t*)realloc(cmd->arg, argAmount * sizeof(char*));
        if (!cmdLine)
        {
            perror("Ivalid command");
            perror(cmdLine);
            exit(errno);
        }
        cmd->arg[cmd->argAmount - 1] = cmdLine;
        cmdLine = strtok(line,"|");
    }

    return cmd;
}

void destructCmd(cmd_t* cmd)
{

}

cmdList_t* constructCmdList(char* line)
{
    cmdList_t* cmdList = (cmdList*)malloc(sizeof(cmdList));
    cmdList->cmdAmount = 0;
    char* cmdLine = strtok(line,"|");
    cmd_t* cmd;

    if (cmdLine == NULL)
    {
        if (!(CHECK_CMD(cmd = constructCmd(line))))
        {
            perror("Ivalid command");
            perror(cmd->name);
            exit(errno);
        }
        cmdAmount++;
        cmdList->cmd = (cmd_t*)realloc(cmdList->cmd, cmdAmount * sizeof(cmd_t*));
        cmdList->cmd[cmdList->cmdAmount - 1] = cmd;
        return cmdList;
    }

    while (cmdLine != NULL)
    {
        cmdAmount++;
        cmdList->cmd = (cmd_t*)realloc(cmdList->cmd, cmdAmount * sizeof(cmd_t*));
        if (!(CHECK_CMD(cmd = constructCmd(line))))
        {
            perror("Ivalid command");
            perror(cmd->name);
            exit(errno);
        }
        cmdList->cmd[cmdList->cmdAmount - 1] = cmd;
        cmdLine = strtok(line,"|");
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