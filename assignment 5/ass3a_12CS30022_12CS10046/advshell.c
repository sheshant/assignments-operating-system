#include <dirent.h> 
#include <stdio.h> 
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <locale.h>
#include <langinfo.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#define MAX_SIZE 1024

//global var.
char c = '\0';
char cwd[1024];
char built_in[][10] = {"cd","pwd","mkdir","rmdir","ls","cp","exit"};

//abhijeet function prototypes
int cd_func(int,char **);
int pwd_func(int ,char **);
int mkdir_func(int ,char **);
int rmdir_func(int ,char **);
int ls_func(int ,char **);
int cp_func(int ,char **);
int exit_func(int ,char **);
int other_func(int ,char **);
int parseAndExec(char* );
void handle_signal(int );

//sheshant function prototypes
void prev_dir();
void copy_paste(char *,char *);
char* get_pwd();
int getModifiedTime(char *);
void change_dir(char *);
void remove_dir(char *);
void make_dir(char *);
void present_dir();
void ls();
int dir_len(char *);
void lss();
mode_t getumask();
int exists(const char *);

//main function
int main(int argc, char *argv[], char *envp[]){	

	//tmp stores whole command
	char tmp[2048];
	bzero(tmp,sizeof(tmp));

	// ctrl+c signal handling
	signal(SIGINT, SIG_IGN);
	signal(SIGINT, handle_signal);

	// current working directory
   	if (getcwd(cwd, sizeof(cwd)) != NULL){

   	}
   	else{
       perror("getcwd() error.");
   	}

   	// shell starts here
	printf("%s>$ ",cwd);
	while(c != EOF) {

		c = getchar();
		switch(c) {
			case '\n': 
				parseAndExec(tmp);
				bzero(tmp, sizeof(tmp));
				printf("%s>$ ",cwd);
				break;
			default:
				strncat(tmp, &c, 1);
				break;
		}
	}
	printf("\n");
	return 0;
}

int cd_func(int m_argc,char **m_argv){
	if(m_argc==0)
		printf("Please enter Directory name \n");
	else
	{
		char * l;
		l = (char*)malloc(5*sizeof(char));
		l = "..";
		if(strcmp(m_argv[m_argc-1], l ) == 0)
		{
			prev_dir();
			getcwd(cwd, sizeof(cwd));
		}
		else 
		{
			change_dir(m_argv[0]);
			getcwd(cwd, sizeof(cwd));
		}
	}
	return 0;
}
int pwd_func(int m_argc,char **m_argv){
	present_dir();
	return 0;
}
int mkdir_func(int m_argc,char **m_argv){
	if(m_argc==0)
		printf("Please enter Directory name \n");
	else
	{
		make_dir(m_argv[0]);
	}
	return 0;
}
int rmdir_func(int m_argc,char **m_argv){
	if(m_argc==0)
		printf("Please enter Directory name \n");
	else
	{
		remove_dir(m_argv[0]);
	}
	return 0;
}
int ls_func(int m_argc,char **m_argv){
	if(m_argc==0){
		ls();
	}
	else{
		char * l;
		l = (char*)malloc(5*sizeof(char));
		l = "-l";
		if(strcmp(m_argv[m_argc-1], l ) == 0)
		{
			lss();
		}
		else {
			perror ("Wrong command.");
		}
	}	
	return 0;
}
int cp_func(int m_argc,char **m_argv){
	if(m_argc !=2)
	{
		printf("Please give proper argument\n");
	}
	else
	{
		copy_paste(m_argv[0],m_argv[1]);
	}
	return 0;
}
int exit_func(int m_argc,char **m_argv){
	exit(0);
	return 1;
}


int parseAndExec(char* ip){

	// defining some variable
	int i,len = strlen(ip);
	char *pch,str[2048];
	
	// copying original ip in str because strtok modifies string input
	bzero(str,sizeof(str));
	strcpy(str,ip);

	// first token - it is command start
	pch = strtok (str," ");
	if(pch == NULL){
		// this means user has pressed some whitespaces and gave the command(pressed enter) !
		return -1;
	}

	// this vector contain command tokens
	int m_argc = 0;
	int n_argc = 0;
	char ** m_argv;
	char ** n_argv;
	m_argv = (char**)malloc(100*sizeof(char*));
	n_argv = (char**)malloc(100*sizeof(char*));
	for(i=0;i<100;i++){
		m_argv[i] = (char*)malloc(100*sizeof(char));
		n_argv[i] = (char*)malloc(100*sizeof(char));
	}

	//copy first token in m_argv[0]
	strcpy(m_argv[m_argc++],pch);

	//copy rest tokens in m_argv[1..n]
	if(pch != NULL){
		while(pch!=NULL){
			pch = strtok (NULL," ");
			if(pch){
				strcpy(m_argv[m_argc++],pch);
			}
		}
	}

	int cmd_start = 0;
	int status = 0;

	
	int fd[2]; 
	int in = STDIN_FILENO,out = STDOUT_FILENO; 
	int saved_out = dup(STDOUT_FILENO);
	int saved_in = dup(STDIN_FILENO);

	//	check and separate acc to "|"
	/**********************************************************************************
	* m_argv =>	./abc.out  param1  param2  param3  |   ./avg.out   param1'	param2'   *												 
	*	   i =>     (0)		(1)		(2)		(3)	  (4)	  (5)	     (6)	  (7)	  *		 
	*			cmd_start							   cmd_start                      *
	**********************************************************************************/
	for(i=0;i<m_argc;i++){

		if(strcmp(m_argv[i],"|")==0 || i==m_argc-1){
			//	pipe
			pipe(fd);

			// 	set variable "out"
			if(i==m_argc-1){ // for last command output will be stdout
				++i;
				out = STDOUT_FILENO;
			}
			else{ // for commands except than last output will be write end of pipe => fd[1]
				out=fd[1];
			}

			// redirect stdin according to "in"
			if (in != STDIN_FILENO){  // if input is not stdin
				dup2 (in, STDIN_FILENO); 
				close (in); 	
			}

			// redirect stdout according to "out"
     		if (out == STDOUT_FILENO){ // this restores output to stdout 
     			dup2(saved_out , STDOUT_FILENO);
     			close(saved_out);
			}
			else{ // this sets output to out => fd[1]
				dup2 (out, STDOUT_FILENO);
				close (out);
			}

			//copy command token part from m_argv to n_argv
			int j;
			n_argc = 0;
			for(j=cmd_start;j<i;j++){
				strcpy(n_argv[n_argc++],m_argv[j]);
			}

			// find out which command it is	
			int found = -1,z;
			int built_in_count = (sizeof(built_in)/sizeof(built_in[0]));
			for(z=0;z<built_in_count;z++){
				if(strcmp(n_argv[0],built_in[z]) == 0){
					found = z;
				}
		
			}

			// call functions accordingly and pass n_argc and n_argv
			
			switch(found){
				case -1:
					
					status = other_func(n_argc,n_argv);
					break;
				case 0:
					
					status = cd_func(n_argc-1,&n_argv[1]);
					break;
				case 1:
					
					status = pwd_func(n_argc-1,&n_argv[1]);
					break;
				case 2:
					
					status = mkdir_func(n_argc-1,&n_argv[1]);
					break;
				case 3:
					
					status = rmdir_func(n_argc-1,&n_argv[1]);
					break;
				case 4:
					
					status = ls_func(n_argc-1,&n_argv[1]);
					break;
				case 5:
					
					status = cp_func(n_argc-1,&n_argv[1]);
					break;
				case 6:
					
					status = exit_func(n_argc-1,&n_argv[1]);
					break;
				default:
					perror("command retrieving error.");
					status = 0;
					break;
				
			}
			/********* command execution returns ********/
			// update cmd_start and "in"
			cmd_start = i+1;
			in = fd[0];

		}	//if ends
	}	//for ends

	//this restores input to stdin
	dup2(saved_in , STDIN_FILENO);
    close(saved_in);
    
	return status;
}


int other_func(int m_argc,char **m_argv){
	if(m_argc==0){
		perror("Invalid command.");
		return 0;
	}

	int i;
	char file_in[100] , file_out[100];
	int in,out;

	pid_t pid ;
	int status;
	char last[2] = "&";
	int background = 0;
	if(strcmp(m_argv[m_argc-1],last)==0){
		m_argv[m_argc-1] = (char*)NULL;
		background = 1;
	}
	else{
		m_argv[m_argc] = (char*)NULL;
	}

	pid = fork();
	if(pid==0){
		if(background==1)
			printf("\n");
		
		// "<" and ">" checking ./a.out param1 param2 < input > output
		for(i=0;i<m_argc;i++){
			if(strcmp(m_argv[i],"<")==0 && m_argv[i+1] !=NULL){
				in = open(m_argv[i+1],O_RDONLY,0);
				dup2(in,STDIN_FILENO);
				close(in);
				m_argv[i] = (char *)NULL;
			}	
			else if(strcmp(m_argv[i],">")==0 && m_argv[i+1] !=NULL){
				int out=open(m_argv[i+1],O_WRONLY|O_CREAT,0666);
				dup2( out, STDOUT_FILENO );
    			close(out);
				m_argv[i] = (char *)NULL;	
			}
		}
		//execute program
		if(execvp(m_argv[0],m_argv) < 0){
			perror("");
			exit(1);
		}
	}
	else{
		if(background == 0){
			while(wait(&status)!=pid)
				;
		}
	}
	return 0;
}

void handle_signal(int signo){
	printf("\n%s>$ ",cwd);
	fflush(stdout);
}

//sheshant function implementation
void prev_dir()
{
	chdir("..");
}

void copy_paste(char *s1,char *s2)
{
	int flag = 0;
	if(exists(s1) == 0)
	{
		printf("File %s doesn't exist\n",s1 );
	}
	else
	{
		if(access(s1,R_OK) != 0)
		{
			printf("File %s doesn't have permission to read\n",s1 );
		}
		else
		{
			if(exists(s2) == 1)
			{
				if(access(s2,W_OK) != 0)
				{
					printf("File %s doesn't have permission to write \n",s2);
				}
				else
				{
					int n1,n2;
					n1 = getModifiedTime(s1);
					n2 = getModifiedTime(s2);
					if(n2<n1)
					{
						printf("modification time for %s is more recent as compared to %s \n",s2,s1);
					}
					else
					{
						flag = 1;
					}
				}
			}
			else
			{
				flag = 1;
			}
		}
	}
	if(flag == 1)
	{
		char c;
		FILE *fptr1, *fptr2;
		fptr1 = fopen(s1,"r");
		fptr2 = fopen(s2,"w");
		c = fgetc(fptr1);
 	   	while (c != EOF)
 	   	{
 	       fputc(c, fptr2);
 	       c = fgetc(fptr1);
 	   	}
		fclose(fptr1);
 	   	fclose(fptr2);
	}
}

char* get_pwd()
{
	char *a;
 	char b[MAX_SIZE];
 	a = getcwd(b,MAX_SIZE);
 	return a;
}

int getModifiedTime(char *s)
{
	struct stat buffer;
 	int         status,n;
 	status = stat(s, &buffer);
 	struct tm *t = localtime(&buffer.st_mtime);
	n = t->tm_hour*3600 + t->tm_min * 60 + t->tm_sec;
	return n ;
}

void change_dir(char *s)
{
	if(exists(s)==1)
	{
		int m = chdir(s);
		if(m == 0){
			//printf("Directory changed to %s \n",s);
		}
		else
			printf("Directory cannot be changed to %s :Permission denied\n",s);
	}
	else
	{
		printf("Directory %s doesn't exist\n",s);
	}
}

void remove_dir(char *s)
{
	int c,d = exists(s);
	if(d == 0)
	{
		printf("Can't remove file doesn't exist\n");
	}
	else
	{
		c = rmdir(s);
		if(c == 0)
		{
			//printf("File %s removed\n",s);
		}
		else
		{
			if(dir_len(s) > 2)
			{
				printf("File %s is not empty and cannot be removed\n",s);
			}
			else
			{
				printf("File %s cannot be removed permission denied\n",s);
			}
		}
	}
}

void make_dir(char *s)
{
	mode_t mask = getumask();
	int c = mkdir(s, 0777);
	if(c == -1)
	{
		if(exists(s) == 1)
			printf("Directory already exist cannot be created\n");
		else
			printf("Directory cannot be created as user doesn't have permission\n");
	}
	else{
		//printf("Directory %s created\n",s);
	}
}
void present_dir()
{
	char b[MAX_SIZE];
	printf("%s\n", getcwd(b,MAX_SIZE));
}

void ls()
{
  
  	DIR           *d;//ino_t  d_ino       file serial number
	struct dirent *dir;
	struct stat buffer;
	char * aa;
	char bb[MAX_SIZE];
 	aa = getcwd(bb,MAX_SIZE);
	d = opendir(aa);
	if (d)
	{
	  		int i=0,c;
	  		while ((dir = readdir(d)) != NULL)  /*it will return null if all the directories are completed*/
		    {
			  	c = dir->d_type;
			  	if(dir->d_name[0] != '.')
			  	{
				  	if(c == 4)
				  	{
				  		printf("\033[1;34m");
				  	}
				  	else 
				  	{
				  		printf("\033[1;31m");
				  	}
				     printf("%-45s", dir->d_name);
				     if(i%3 ==2)
				     	printf("\n");
				     i++;
				 }
		    }
		    printf("\033[0m\n");

	    closedir(d);
	}
}

int dir_len(char * s)
{
	int i =0;
	char *a;
	char *f = "/";
 	char b[MAX_SIZE];
 	a = getcwd(b,MAX_SIZE);
 	strcat(a,f);
 	strcat(a,s);
 	DIR           *d;
    struct dirent *dir;
 	d = opendir(a);
 	if(d){
 		while ((dir = readdir(d)) != NULL) {
 			i++;
 		}
 		closedir(d);
 	}
 	else{
 		i = -1;
 		printf("Directory doesn't exist\n");
 	}
 	return i;
}


void lss()
{
	
	DIR           *d;
	struct dirent *dir;
	struct stat buffer;
	char * aa;
	char bb[MAX_SIZE];
 	aa = getcwd(bb,MAX_SIZE);
	d = opendir(aa);
   if (d)
   {
   		int c;
   		while ((dir = readdir(d)) != NULL)  /*it will return null if all the directories are completed*/
 	    {
 		  	c = dir->d_type;
 		  	if(dir->d_name[0] != '.')
 		  	{
 		  		char *a;
 		  		char *f = "/";
			 	char b[MAX_SIZE];
			 	a = getcwd(b,MAX_SIZE);
			 	strcat(a,f);
			 	strcat(a,dir->d_name);
 				int         status;
 				status = stat(a, &buffer);
 				struct passwd *pw = getpwuid(buffer.st_uid);
				struct group  *gr = getgrgid(buffer.st_gid);
				printf( (S_ISDIR(buffer.st_mode)) ? "d" : "-");
			    printf( (buffer.st_mode & S_IRUSR) ? "r" : "-");//read permission for user
			    printf( (buffer.st_mode & S_IWUSR) ? "w" : "-");// write permission for user 
			    printf( (buffer.st_mode & S_IXUSR) ? "x" : "-");
			    printf( (buffer.st_mode & S_IRGRP) ? "r" : "-");
			    printf( (buffer.st_mode & S_IWGRP) ? "w" : "-");
			    printf( (buffer.st_mode & S_IXGRP) ? "x" : "-");
			    printf( (buffer.st_mode & S_IROTH) ? "r" : "-");
			    printf( (buffer.st_mode & S_IWOTH) ? "w" : "-");
			    printf( (buffer.st_mode & S_IXOTH) ? "x" : "-");
			    printf("  ");
			    printf("%ld", (long) buffer.st_nlink);
			    printf(" %s ",  pw->pw_name);
			    printf(" %s ",  gr->gr_name);
			    printf("%10lld bytes",(long long) buffer.st_size);
			    if(c == 4)
			    	printf("\033[1;34m");
			    else
			    	printf("\033[1;31m");
			    printf(": %-40s : ",dir->d_name);
			    printf("\033[0m");
			    printf(" %24s ",ctime(&buffer.st_mtime));
 			 }
 	    }
     closedir(d);
   }
}

mode_t getumask()
{
    mode_t mask = umask(0);
    umask (mask);
    return mask;
}

int exists(const char *fname)
{
    if( access( fname, F_OK ) != -1 ) 
    {//file exists 
    	return 1; 
	} 
	else 
	{//file doesn't exist
	    return 0; 
	}
}
