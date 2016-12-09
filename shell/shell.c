#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define LEN_NAME     100
#define LEN_ARG      100
#define LEN_COMMAND  100
#define LEN_BUF      1000

#define NUM_ARG 20*sizeof(void*)
#define NUM_CMD 20*sizeof(void*)

#define quit          0
#define fail_exec     1
#define success_exec  2

#define BYTES(bytes) \
	WEXITSTATUS(bytes);

#define UNKNOWN_CMD 	"I don't know this command!\n"
#define EXIT 		"exit"


#define NOT_OK \
	exec->OK = FALSE; \
	return;

#define IS_IT_ERROR(condition, note, punishment) \
	if(condition){ \
		if(note) fprintf(stderr, "%s. Error: %s\n", note, strerror(errno)); \
		punishment; \
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
	printf("}\n");\
	printf("command = \"%s\"\n", command);
	
#define M printf("Mitrikas\n");

struct cmd_t{
	char* name;
	char** arg;
	int OK;
};

void printMan()
{
    printf("==================.:Hello, my Dear Friend:.=====================\n"); \
    printf("Write your commands in form of 'cmd(1) | cmd(2) ... | cmd(n)'...\n"); \
    printf("--------------------(type '%s' to quite)----------------------\n", EXIT)
}

int cmd_OK(struct cmd_t* exec){

	 return exec->OK &&
		exec->name &&
		exec->arg;
}

void cmd_ctor(struct cmd_t* exec){

	exec->name = (char*)malloc(LEN_NAME);
	IS_IT_ERROR(!exec->name, "Can't allocate memmory", NOT_OK);

	exec->arg = malloc(NUM_ARG);
	IS_IT_ERROR(!exec->arg, "Can't allocate memmory", NOT_OK);

	int num_arg = 0;
	while(num_arg < NUM_ARG){
		exec->arg[num_arg] = (char*)malloc(LEN_ARG);
		IS_IT_ERROR(!exec->arg[num_arg], "Can't allocate memmory", NOT_OK);
		num_arg++;
	}

	exec->OK = TRUE;
}

void cmd_dtor(struct cmd_t* exec){

	free(exec->name);

	int num_arg = 0;
	while(num_arg < NUM_ARG){
		if(exec->arg[num_arg]) free(exec->arg[num_arg]);
		num_arg++;
	}

	free(exec->arg);

	exec->OK = FALSE;
}


void close_pipe   (int    Pipe[2]					   		     );
void get_command  (char** command							     );
int  exec_command (char*  command							     );
void command_OK   (char*  command,     int*   OK 					     );
void make_cmd     (struct cmd_t* exec, char** command,     int* Num_arg			     );
int  exec_cmd     (struct cmd_t* exec, int    num_cmd,     int  pipe_prev[2], int pipe_cur[2]);
void renew 	  (struct cmd_t* exec, int    num_arg					     );
void unknown	  (char* string,       int*   unknown_cmd, int  size			     );
int  count_bytes  (struct cmd_t* exec, int    Pipe[2],     int  brother			     );
int  over         (struct cmd_t* exec, char*  command,     int  ret                          );

char ERROR_NOTE[2000];


int main(void){

	printMan;

	char* command;
	int ret;

	while(1){
		get_command(&command);
		ret = exec_command(command);
		switch(ret){
			case quit: 	   return  0;
			case fail_exec:	   return -1;
			case success_exec: continue;
		}
	}
}

void get_command(char** command){
	
	char* Command = (char*)malloc(LEN_COMMAND);
	if(!Command){
		*command = NULL;
		return;
	}

	printf("command: ");
	fgets(Command, LEN_COMMAND, stdin);
	int OK;
	command_OK(Command, &OK);
	switch(OK){
		case TRUE:  Command[strlen(Command)-1] = 0;
			    break;
		case FALSE: Command = NULL;
			    break;
	}

	*command = Command;
}

void command_OK(char* command, int* OK){

	if(!command){
		printf("Can't get command from console. Error: %s", strerror(errno));
		*OK = FALSE;
		return;
	}

	if(!strchr(command, '\n')){
		printf("Command line is too long. It should be less than %d\n", LEN_COMMAND);
		*OK = FALSE;
		return;
	}

	if(!*command){
		*OK = FALSE;
		return;
	}

	*OK = TRUE;
}

int exec_command(char* command){

	if(!command) return quit;
	if(!strcmp(command, EXIT)) return quit;

	struct cmd_t exec;
	cmd_ctor(&exec);

	int Pipe[NUM_CMD][2];

	int num_cmd = 0;
	int num_arg = 0;
	int ret;
	int pid;

	int cnt = 0;

	while(1){
		make_cmd(&exec, &command, &num_arg);

		pid = pipe(Pipe[num_cmd]);
		ret = exec_cmd(&exec, num_cmd, Pipe[num_cmd-1], Pipe[num_cmd]);

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

void make_cmd(struct cmd_t* exec, char** command, int* Num_arg){

	exec->OK = FALSE;

	char Command[LEN_COMMAND];
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

int exec_cmd(struct cmd_t* exec, int num_cmd, int pipe_prev[2], int pipe_cur[2]){

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