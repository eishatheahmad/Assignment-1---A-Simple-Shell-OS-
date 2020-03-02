//17l-4651
//Operating Systems
//Assignment 1


#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>


#define input_mode 0
#define output_mode 1


char current_directory[1000];
char input_buffer[1024];
char cwd[1024];
int flag;
int len;



//initial screen which loads the current directory, and $ sign
void initiliaze()//prompt
{

	char environment[1000];
	if(getcwd(cwd,sizeof(cwd))!=NULL)
	{

		strcpy(environment,cwd);
		strcat(environment,"$ ");
		printf("%s",environment);


	}


	else
	{
		perror("getcwd() error");
	}
}

//reset variables for use 
void clearVariables()
{
	input_buffer[0]='\0';
	cwd[0] = '\0';
	flag=0;
	len=0;

}


void removeSpaces(char *str)
{
	int l=strlen(str)-1;
	if( l==' ' || l=='\n')
	{
		str[l]='\0';
	}

	if(str[0]==' ' || str[0]=='\n')
	
	{
		memmove(str,str+1,strlen(str));/* Copies contents of str2 to sr1 */
	}

}

//create tokens of the command arguments on basis of spaces, pipe etc in const char 
//variable and return array of tokens in char** and size of the array in int*
void tokenize(char **arg, int *ptr, char *buff, const char*d)
{

	char *token;
	token=strtok(buff,d);
	int temp=-1; //iterator to measure length

	while(token)
	{
		arg[++temp]=malloc(sizeof(token)+1);
		strcpy(arg[temp],token);
		removeSpaces(arg[temp]);
		token=strtok(NULL,d); //tokenize and split string into tokens


	}
	arg[++temp]=NULL;
	*ptr=temp;


}

//execute pipes passed in cmd that are piped 
void executePiped(char** str,int count){
	if(count>2) 
	{
		return;
	}
	int fd[2][2];
	int i=0;
	int temp=0;
	char *argv[100];

	for(i=0;i<count;i++)
	{
		tokenize(argv,&temp,str[i]," ");
		if(i!=count-1) //[count] would be for writing
		{
			if(pipe(fd[i])<0)
			{
				perror("ERROR! IN EXECUTE PIPED. PIPE NOT CREATED\n");
				return;
			}
		}
		if(fork()==0) //for child
		{
		
			if(i!=count-1)
			{
				dup2(fd[i][1],1);
				close(fd[i][0]);
				close(fd[i][1]);
			}

			if(i!=0){
				dup2(fd[i-1][0],0); //int dup2(int oldfd, int newfd);
				close(fd[i-1][1]);
				close(fd[i-1][0]);
			}
			execvp(argv[0],argv);
			perror("invalid input ");
			exit(1);//in case exec is not successfull, exit
		}
		//parent
		if(i!=0){//second process
			close(fd[i-1][0]);
			close(fd[i-1][1]);
		}
		wait(NULL);
	}
}

//execute one string commands like ls etc 
void single_commands(char **input)
{
	if(fork()>0) //parent
	{
		wait(NULL);
	}

	else
	{
		execvp(input[0],input);
		perror("INVALID INPUT (in single commands");
		exit(1);

	}

}



void redirectionCommand(char **buffer, int count, int mode)
{
	int temp,fd;
	char*argv[100];
	removeSpaces(buffer[1]);

	tokenize(argv,&temp,buffer[0]," ");


	if(fork()==0)
	{
		switch(mode)
		{
			case output_mode: 
					fd=open(buffer[1],O_WRONLY);// Open for writing only
					break;
			default: return;

		}
		

	
		if(fd<0)
		{
			perror("cannot open file\n");
			return;
		}

		switch(mode)
		{
			case output_mode: 
					dup2(fd,1);
					break;
			default: return;

		}

		execvp(argv[0],argv);
		perror("INVALID INPUT IN REDIRECT");
		exit(1);
	}
	wait(NULL);



}


int main()
{
	char *args1[100];
	int count=0;
	char next_line[2]={"\n"};
	char *buffer[100];
	getcwd(current_directory, sizeof(current_directory));
	//printf(current_directory);

	while(1)
	{
		clearVariables();
		initiliaze();
		fgets(input_buffer,1024,stdin);


//if hitting enter just continue the loop 
		if(strcmp(input_buffer,next_line)==0)
		{
			//printf("1n");
			continue;
		}


//tokenize for exit
		len=strlen(input_buffer);
		input_buffer[len-1]='\0';


//check for pipe
		if(strchr(input_buffer,'|'))
		{
			tokenize(buffer,&count,input_buffer,"|");
			executePiped(buffer,count);

		}

		if(strchr(input_buffer,'>'))
		{
			tokenize(buffer,&count,input_buffer,">");
			if(count==2)
			{
				redirectionCommand(buffer,count,output_mode);
			}
			else
			{
				printf("INCOREECT REDIRECTION. has to to be in this form: command > file)");
			}

		}


		if(strcmp(input_buffer,"exit")==0)
		{
//printf("2n");
			flag=1;
			break;

		}


		else
		{
			tokenize(args1,&count,input_buffer," ");

			if(strstr(args1[0],"cd"))
			{
				if(chdir(args1[1])==-1)
				{
					printf("bash: cd: %s: No such file or directory\n",args1[1]);


				}

				else
				{
					chdir(args1[1]);

				}

			}

		else
		{

			single_commands(args1);

		}


	}


	}

	if(flag==1)
	{
		printf("EXITTING SHELL...\n");
		exit(0);
		return 0;
	}
	
	return 0;

}