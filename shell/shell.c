#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>
#include <fcntl.h>
#include <string.h>

#define LEN_NAME     100
#define LEN_ARG      100
#define LEN_BUF      1000

#define EXEC_FAIL     -1
#define QUIT          0
#define EXEC_SUCCESS  1

#define BYTES(bytes) \
	WEXITSTATUS(bytes);

#define UNKNOWN_CMD "Unknown command\n"

#define NOT_OK \
	exec->OK = FALSE; \ 
	return;

#define CMD_IS_OK(exec) exec->OK && exec->name && exec->arg;

#define IS_IT_ERROR(condition, note, action) \
	if (condition) \
    { \
		if (note) fprintf(stderr, "%s. Error: %s\n", note, strerror(errno)); \
		action; \
	}

#define dump(exec) \
	printf("struct cmd_t \"exec\"{\n"); \
	printf("	exec->name = \"%s\";\n", exec.name); \
	cnt = 0; \
	while(exec.arg[cnt]){ \
		printf("	exec->arg[%d] = \"%s\";\n", cnt, exec.arg[cnt]); \
		cnt++; \
	} \
	switch(exec.OK){ \
		case TRUE:  printf("	exec->OK = TRUE\n"); \
			    break; \
		case FALSE: printf("	exec->OK = FALSE\n"); \
			    break; \
	} \

typedef struct cmd_t{
	char* name;
	char** arg;
    size_t argAmount;
	int OK;
}
cmd_t;

void printManual()
{
    printf("Here manual wanna be\n");
}

cmd_t* constructCmd()
{
    cmd_t *exec = (char*)malloc(sizeof(cmd_t));
	exec->name = NULL;
	exec->arg = NULL;
    argAmount = 0;
	exec->OK = FALSE;
    return exec;
}

void destructCmd(cmd_t* exec)
{
	free(exec->name);
	for (int i = 0; i < argAmount; i++)
	{
        free(exec->arg[i]);
	}
	free(exec->arg);
    argAmount = 0;
	exec->OK = FALSE;
    free(exec);
}



char* getLine();
int  execLine(char* line);
void makeCmd(cmd_t* exec, char** line, int* Num_arg);
int  executeCmd(cmd_t* exec, int num_cmd, int  pipe_prev[2], int pipe_cur[2]);
void renew(cmd_t* exec, int num_arg);
void unknown(char* string, int* unknown_cmd, int size);
void close_pipe(int Pipe[2]);
int  count_bytes(cmd_t* exec, int Pipe[2], int  brother);
int  over(cmd_t* exec, char* line, int  ret);

char ERROR_NOTE[2000];


int main(int argc, char** argv, char** envp)
{
	printManual();

    char line[MAX_LINE_LENGTH];

	while(TRUE)
    {
        if (!line)
        {
		    printf("Can't get command line from console. Error occured: %s", strerror(errno));
		    return 0;
	    }
		int ret = execLine(line);
		switch(ret)
        {
			case QUIT:
                return  0;
                break;
			case EXEC_FAIL:
                return -1;
                break;
            default:
                break;
		}
	}
}

void getLine(char* line)
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

int execLine(char* line)
{
	if(!strcmp(command, EXIT)) return QUIT;

	cmd_t *exec = constructCmd();

	int Pipe[NUM_CMD][2];

	int num_cmd = 0;
	int num_arg = 0;
	int ret;
	int pid;

	int cnt = 0;

	while(1){
		makeCmd(&exec, &line, &num_arg);

		pid = pipe(Pipe[num_cmd]);
		ret = executeCmd(&exec, num_cmd, Pipe[num_cmd-1], Pipe[num_cmd]);

		if(num_cmd) close_pipe(Pipe[num_cmd-1]);
		if(command) wait(&ret);

		num_cmd++;
		if(over(&exec, command, ret)) break;
		renew(&exec, num_arg);
	}

	ret = count_bytes(&exec, Pipe[num_cmd-1], pid);
	printf("\n");
	
	return ret;
}	

void makeCmd(struct cmd_t* exec, char** line, int* Num_arg){

	exec->OK = FALSE;

	char Command[MAX_LINE_LENGTH];
	strcpy(Command, *command);

	char* token;

	int num_arg = 0;
	
	int offset = 0;

	while(1){
		if(!num_arg){
			token = strtok(Command, " ");
			if(!token) break;
			strcpy(exec->name, token);
			strcpy(exec->arg[0], exec->name);
		}
		else{
			token = strtok(NULL, " ");
			if(!token) break;
			strcpy(exec->arg[num_arg], token);
			if(num_arg > 1)offset++;
		}

		if(!strcmp(exec->arg[num_arg], "|")){
			offset += 3;
			break;
		}
		else{
			offset += strlen(exec->arg[num_arg]);
		}
	
		num_arg++;
		if(2*num_arg > NUM_ARG){
			fprintf(stderr, "To much arguments for cmd \"%s\". It should be less than %d\n", exec->name, NUM_ARG);
			exec->OK = FALSE;
			return;
		}
	}

	*Num_arg = num_arg;

	if(token){
		exec->arg[num_arg] = NULL;
		*command += offset;
	}
	else{
		exec->arg[num_arg] = NULL;                          
		*command = NULL;
	}

	exec->OK = TRUE;
}

int executeCmd(struct cmd_t* exec, int num_cmd, int pipe_prev[2], int pipe_cur[2]){

	if(exec->OK == FALSE) return -1;

	int child = fork();
	IS_IT_ERROR(child < 0, "Can't create child process", return -1);

	if(!child){
		if(num_cmd){
			dup2(pipe_prev[0], 0);
			close_pipe(pipe_prev);
		}

		dup2(pipe_cur[1], 1);
		close_pipe(pipe_cur);

		execvp(exec->name, exec->arg);

		strcpy(ERROR_NOTE, strerror(errno));
		printf("%s", ERROR_NOTE);
	
		exit(13);
	}

	return 0;
}

void close_pipe(int Pipe[2]){

	close(Pipe[0]);
	close(Pipe[1]);
}


int count_bytes(struct cmd_t* exec, int Pipe[2], int brother){

	if(exec->OK == FALSE) return quit;

	int bytes = 0;

	int child = fork();
	IS_IT_ERROR(child < 0, "Can't create child process", return fail_exec);

	if(!child){
		dup2(Pipe[0], 0);
		close_pipe(Pipe);

		char buf[LEN_BUF];
		int rd, wr;

		int unknown_cmd = FALSE;

		while(1){
			rd = read(0, buf, LEN_BUF);
			if(!rd){
				if(bytes > 0 && unknown_cmd == FALSE) printf("(%d bytes)\n", bytes);
				exit(success_exec);
			}

			IS_IT_ERROR(rd < 0, "Can't read from pipe", exit(fail_exec));

			if(!bytes) unknown(buf, &unknown_cmd, rd);

			wr = write(1, buf, rd);
			IS_IT_ERROR(rd < 0, "Can't write to console", exit(fail_exec));

			bytes += rd;
		}
	}

	close_pipe(Pipe);

	int ret;
	waitpid(brother, &ret, 0);
	waitpid(child,   &ret, 0);
	return WEXITSTATUS(ret);
}

void unknown(char* string, int* unknown_cmd, int size){

	if(!strncmp(string, ERROR_NOTE, size)){
		*unknown_cmd = TRUE;
		return;
	}
	else{
		printf("output:\n");
		return;
	}
}

int over(struct cmd_t* exec, char* command, int ret){
	
	if(exec->OK == FALSE){
		return TRUE;
	}

	if(WEXITSTATUS(ret) == 13){
		return TRUE;
	}

	if(WEXITSTATUS(ret)){
		printf("Fail execution of command \"%s\"\n", exec-> name);
		return TRUE;
	}

	if(!command){
		return TRUE;
	}

	return FALSE;
}

void renew(struct cmd_t* exec, int num_arg){

		exec->arg[num_arg] = (char*)malloc(LEN_ARG);
		IS_IT_ERROR(!exec->arg[num_arg], "Can't allocate memmory", NOT_OK);
}