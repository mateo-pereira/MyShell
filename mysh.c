#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>

int preverror = 0;
int cmderror = 0;
int argsize;



typedef struct process{
    char* exec;
   	 char* input;
   	 char* output;
   	 int argc;
   	 int ifPipe;
   	 char **arguments;
   	 struct process *next;
} Process;

int mycd(Process *);
int myexit(Process *);
int mypwd(Process *);

char *mycmdlist[] =
{
    "cd",
    "exit",
    "pwd"
};

int (*mycmds[])(Process *) =
{
    &mycd,
    &myexit,
    &mypwd
};



int numcmds()
{
    return sizeof(mycmdlist)/sizeof(char *);
}

int mycd(Process *command)
{
    char *home = getenv("HOME");
    if(command->argc == 1)
    {
   	 if(chdir(home) != 0)
   	 {
   		 perror("mysh");
   		 preverror = 1;
   		 return 1;
   	 }
    }
    else
    {
   	 if(chdir(command->arguments[1]) != 0)
   	 {
   		 perror("mysh");
   		 preverror = 1;
   	 }
    }
    
    return 1;
}
int mypwd(Process *command)
{
    char cwd[128];
    if(getcwd(cwd, sizeof(cwd)) != NULL)
    {    write(STDOUT_FILENO, cwd, strlen(cwd)); write(STDOUT_FILENO, "\n", 1);  }
    else
    {
    	
	perror("mysh pwd error");
   	preverror = 1;  
    }
    
    return 1;
}
int myexit(Process *command)
{
    
    return 0;
}


char** getArgs()
{
	int i;
	char *input = malloc(128*sizeof(char));
	int size = 1;
	int count = 0;
	char buffer[128];
	int bytes;
	int breaker = 0;
	int row, col, words, max_char_length;

	words = 1;
	max_char_length = 0;
	int char_length = 0;




	while ((bytes = read(STDIN_FILENO,buffer,128)) > 0 && breaker == 0)
        {
            for (i = 0;i < bytes;i++)
            {
                if (max_char_length < char_length) max_char_length = char_length;

                if (buffer[i] == '\n')
                {
                    breaker = 1;
                    break;
                }
                
                if (buffer[i] == ' ')
                {
                    words++;
                    i++;
                    char_length = 0;
                    while (buffer[i] == ' ') //Checks to see if there is an extra space, if there is skip it
                    {
                        if (i >= bytes) break;
                        i++;
                    }
                    i--;
                }
                if (buffer[i] == '>' || buffer[i] == '<' || buffer[i] == '|')
                {
                    if (buffer[i-1] != ' ' && buffer[i+1] != ' ') words += 2;
                    else if (buffer[i-1] != ' ') words++; //Could go out of bounds
                    else if (buffer[i+1] != ' ') words ++;
                
                    char_length = 0;
                    input[count++] = buffer[i];

                    if (count==size)
                        input = realloc(input, sizeof(*input)*(size+=16)); //Creates more space for input if needed 
                    continue;
                }

                input[count++] = buffer[i]; //Copies buffer[i] into input, then increments
                char_length++;
                
                if (count==size)
                    input = realloc(input, sizeof(*input)*(size+=16)); 
                
            }
            if(breaker == 1)
                	break;
        }
    	input[count] = '\0';
   	 
    	for (i = 0;i<strlen(input);i++) //Error cases for input
    	{
        	if (input[i] == '>' || input[i] == '<' || input[i] == '|')
        	{
		    	if ((i+1) > strlen(input)) //Error if there is nothing after < > |
		    	{
		        	perror("Nothing after < > | error");
		        	preverror = 1;
		       	 
		    	}
		    	if (input[i+1] == '>' || input[i+1] == '<' || input[i+1] == '|') //Error if there is a repeating < > or | after an already existing <> |
		    	{
		        	perror(" < > | after an already existing < > | error");
		        	preverror = 1;
		       	 
		    	}
        	}
    	}
    char **tokenArr = (char**)malloc(words*sizeof(char*)); // 2D array allocated
    	for (i = 0;i< words;i++)
        	tokenArr[i] = malloc(max_char_length * sizeof(char)+1);

    	row = 0;
    	col = 0;
    	for (i = 0;i < strlen(input);i++) //Puts everything from input into 2D array (every row is a word)
        {
            if (input[i] == ' ')
            {
                tokenArr[row][col] = '\0';
                
                row++;
                col = 0;
                
            }
           
            else if (input[i] == '>' || input[i] == '<' || input[i] == '|')
            {
                if (input[i-1] == ' ')
                {
                    col = 0;
                    
                    tokenArr[row][col] = input[i];
                    tokenArr[row][col+1] = '\0';
                    if (input[i+1] != ' ') row++;
                    printf("%c xD",tokenArr[2][0]);
                }
                else if((input[i-1] != ' '))
                {
                    row++;
                    col = 0;
                    tokenArr[row][col] = input[i];
                    tokenArr[row][col+1] = '\0';
                    if (input[i+1] != ' ') row++;
                    col = 0;
                }
            }
            else
            {
                
                tokenArr[row][col] = input[i];
                col++;
            }
                
        }
    	tokenArr[row][col] = '\0';
   	 
   	 argsize = words;
   	 

   	 

    free(input);
    return tokenArr;
}

void redirectIn(char * fileName)
{
    if(fileName == NULL)
    {
   	 write(STDERR_FILENO, "redirects to nothing\n", 21);
   	 preverror = 1;
   	 return;
    }
    
    int in = open(fileName, O_RDONLY);
    if(in == -1)
    {
   	 perror("mysh rdi file error:");
   	 preverror = 1;
    }
    if(dup2(in, 0) == -1)
    {
   	 perror("mysh rddup2:");
   	 preverror = 1;
    }
    
    close(in);
}

void redirectOut(char *fileName)
{    
    
    if(fileName == NULL)
    {
   	 write(STDERR_FILENO, "redirects to nothing\n", 21);
   	 preverror = 1;
   	 return;
    }
    int out = open(fileName, O_WRONLY | O_TRUNC| O_CREAT, 0640);
    if(out == -1)
    {
   	 perror("mysh rdo file error:");
   	 preverror = 1;
    }
    if(dup2(out, 1) == -1)
    {
   	 perror("mysh rddup2:");
   	 preverror = 1;
    }
    close(out);
}

int runReg(Process *cmd)
{    
    int pid = fork();
    int status;
    
    
    if(cmd->input != NULL)
    {
   	 redirectIn(cmd->input);
    }
    if(cmd->output != NULL)
    {
   	 redirectOut(cmd->output);
    }
    
    if(pid < 0) // when fork() fails
    {
   	 preverror = 1;
   	 perror("mysh");
    }
    else if(pid ==0) // child
    {
   	 if(execvp(cmd->exec, cmd->arguments) < 0)
   	 {
   	 	
   		 perror("exec failed");    
   		 exit(EXIT_FAILURE);
   	 }
    }
    else
    {
   	 
   		 waitpid(pid, &status, 0);
   		 if(status != 0)
   			 preverror = 1;
   	 
   	 
   	 
    }
    return 1; // success
    
}

int runPiped(Process *head)
{
    Process *ptr = head;
    int pipefd[2];
    int p1, p2;
    int p1status, p2status;
    
    if(pipe(pipefd) < 0 )
    {
   	 perror("mysh pipe error");
   	 preverror = 1;
    }
    
    if((p1 = fork()) < 0)
    {
   	 perror("mysh pfork error:");
   	 preverror = 1;
    }
    
    if(p1 == 0) // child 1executing
    {
   	 // only needs to write at write end;
   	 close(pipefd[0]);
   	 dup2(pipefd[1], STDOUT_FILENO);
   	 close(pipefd[1]);
   	 
   	 if(execvp(ptr->exec, ptr->arguments) < 0)
   	 {
   		 perror("mysh pexec error:");
   		 exit(EXIT_FAILURE);
   	 }
    }
    
    else // parent 1 executing
    {
   	 p2 = fork();
   	 if(p2 < 0)
   	 {
   		 perror("mysh p1fork error:");
   		 preverror = 1;
   	 }
   	 
   	 if(p2 == 0) // child 2 executing
   	 {
   		 close(pipefd[1]);
   		 dup2(pipefd[0], STDIN_FILENO);
   		 close(pipefd[0]);
   		 
   		 if(execvp(ptr->exec, ptr->arguments) < 0)
   		 {
   			 perror("mysh pexec error:");
   			 exit(EXIT_FAILURE);
   		 }
   	 }
   	 
   	 else
   	 {
   		 do {    
   			 waitpid(p1, &p1status, 0);
   			 waitpid(p2, &p2status, 0);
   			 if((WIFEXITED(p1status) && p1status != 0) || (WIFEXITED(p2status) && p2status))
   				 preverror = 1;
   		 } while (!WIFEXITED(p1status) && !WIFSIGNALED(p1status) && !WIFEXITED(p2status) && !WIFSIGNALED(p2status));
   	 }
    }
    
    return 1; // success
}

int execute(Process *head)
{
    Process *cmdptr = head;
    int i;
    if(cmdptr->exec == NULL) // empty command
    {
   	 write(STDERR_FILENO, "empty command\n", 14);
   	 return 1;
    }
    
    
    // we check for pipes first to see if multiple commands, else theres only one command
    if(cmdptr->ifPipe == 1)
   	 return runPiped(head);
    
    for(i = 0;  i < numcmds(); i++)
    {
   	 if(strcmp(cmdptr->exec, mycmdlist[i]) == 0)
   	 {
   		 return (*mycmds[i])(cmdptr);
   	 }
    }
    
    
    
    
    return runReg(cmdptr);
    
}

void freeProcess(Process *head)
{
    int i;
    Process* ptr;
    while(head!=NULL)
    {
   	 ptr = head;
   	 head = head->next;
   	 free(ptr->arguments);
   	 free(ptr);
   	 
    }
}

Process *createCmd(void)
{
    Process *temp =  malloc(sizeof(Process));
    temp->exec = NULL;
   	 temp->input = NULL;
   	 temp->output = NULL;
   	 temp->argc = 0;
   	 temp->ifPipe = 0;
   	 temp-> arguments = NULL;
    temp->next = NULL;
    return temp;
}

int checkCommand(Process *cmd)
{
    if(cmd->exec == NULL)
   	 return -1;
    else if(cmd->argc < 1)
   	 return -1;
    else
   	 return 1;
}

Process *getcmdnext(Process *head)
{
    Process *ptr = head;
    while(ptr->next != NULL)
    {
   	 ptr = ptr->next;
    }
    return ptr;
}
/*
int haswild(char *str)
{
    while(i
}

void wildcard(char *arg, char *str, char *end, Process *cmd)
{
    DIR *dir;
    struct dirent *direc;
    dir = opendir(arg);
    int i;
    int indir = 1;
    if(dir != NULL)
    {
   	 indir = 1;
   	 direc = readdir(dir);
   	 while(direc!=NULL)
   	 {
   		 indir = 1;
   		 int index = strlen(direc->d_name) - (strlen(end));
   		 
   		 for(i = 0; i < strlen(end);i++)
   		 {
   			 if(end[i] != direc ->d_name[index])
   				 indir = 0;
   			 index++;
   		 }
   		 if((strncmp(direct->d_name, str, strlen(str)) == 0 && indir)
   		 {
   			 char *string = (char *)malloc(sizeof(char) * (strlen(direc->d_name) + 1));
   			 strcpy(string, direc -> d_name);
   			 
   		 }
   		 direc = readdir(dir);
   	 }
   	 closedir(dir);
    }
    
    else
   	 return;
}
*/
Process *createCommandList(char** args)
{
    int i, numCom;
    numCom = 0;
    Process *commandshead = createCmd();
    Process *command  = commandshead;
    
    command->exec = args[0];
    
    command->arguments = realloc(command->arguments, argsize * sizeof(char*));
    command->arguments[0] = args[0];
    int argcount = 1;
    int check = 0;
    for(i = 1; i < argsize;i++) // need to check if < is at the end of a thingy since we doing i+1
    {
   	 if( i >= argsize )
   	 	break;
   	 if(check == 1) // pipe
   	 {
		numCom++;
		command->ifPipe = 1;
		commandshead = realloc(commandshead, sizeof(Process) * numCom+1);
		command = getcmdnext(commandshead);
		command -> argc = argcount;
		argcount = 0;
		command->exec = args[i+1];
		command->arguments = realloc(command->arguments, argsize * sizeof(char*));
		command->arguments[argcount] = args[i+1];
		argcount++;
		check = 1;
		
   	 }
   	 
   	 if(check == 2) // output
   	 {
   	 	check = 0;
   	 	command->output = args[i];
   	 	
   	 }
   	 if(check == 3) // input
   	 {
   	 	check = 0;
   	 	command->input = args[i];
   	 	
   	 }
   	 
   	 if(strcmp(args[i], "|") == 0)
   	 {
   		 // if i+1 is out of bounds check
   		 
   		 if(checkCommand(command) == 1)
   		 {
   		 	 check = 1;
   		 }
   		 else
   		 {
   			 fprintf(stderr, "\npipe without one cmd or without an argument");
   			 return NULL;
   		 }
   		 
   	 }
   	 //fprintf(stdout, "\n args[i][0]%s\n", args[i-1]); // this was to check for < > working
   	 if(args[i][0] == '>') // output
   	 {
   	 	
   		 // if i+1 is out of bounds check
   		 // NEED
   		 check = 2;
   		 
   		 
   	 }
   	 if(args[i][0] == '<') // input
   	 {
   	 	
   		 // if i+1 is out of bounds check
   		 // NEED
   		 check = 3;
   		 
   		 
   	 }
   	 if(check == 0)
   	 {
   	 		
   		 command->arguments = realloc(command->arguments, argsize * sizeof(char*));
   		 //fprintf(stdout, "\n regrun:%s \n\n", command->arguments[i]);
   		 command->arguments[argcount] = args[i];
   		 argcount++;
   		 
   	 }
   	 
    
    }
    command -> argc = argcount;
    
    return commandshead;
}

void interactive(void)
{
	char **args;
	int status, i;
	do  {
    	if(preverror == 1)
        	{ write(STDOUT_FILENO, "!mysh> ",7); preverror = 0; }
    	else
        	write(STDOUT_FILENO, "mysh> ",6);
    
    	args = getArgs();  
    	//fprintf(stdout, "\n argcount%c \n", args[2][0]);
    	Process *commands = createCommandList(args);
   	
    	
    	if(cmderror == 1)
    	{
   		 preverror =1;
   		 cmderror = 0;
   		 continue;
    	}
    	
    	status = execute(commands);
   	 
   	 for(i = 0; i < argsize; i++)
    	{
   		 free(args[i]);
    	}
    	free(args);
    	freeProcess(commands);
   	 
	}while(status);
}

/*
void batch(char **argv)
{
    if(argv[1] == NULL)
    {
   	 printf("Running Interactive Mode Due to Empty file");
   	 interactive();
    }
    
    int in = open(argv[1], O_RDONLY);
    char **args;
    while((args = getArgs()) != NULL || getArgs() != eof)
    {
   	 
    }
    
}
*/



int main(int argc, char **argv)
{
	printf("Welcome to my shell!\n");
	if(argc == 1)
    	interactive();
	//else
  	//  batch(argv);


	return EXIT_SUCCESS;
}