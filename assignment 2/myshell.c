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
#define MAX_SIZE 100

char * get_pwd()
{
	char *a;
 	char b[MAX_SIZE];
 	a = getcwd(b,MAX_SIZE);
 	return a;
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
void exiting()
{
	exit(0);
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
void prev_dir()
{
	chdir("..");
}

char c = '\0';
char cwd[1024];
char built_in[][10] = {"cd","pwd","mkdir","rmdir","ls","cp","exit"};
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

int main(int argc, char *argv[], char *envp[]){	

	//tmp stores whole command
	char tmp[1024];
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
	exiting();
	return 1;
}
int other_func(int m_argc,char **m_argv){
	if(m_argc==0){
		perror("Invalid command.");
		return -1;
	}
	pid_t pid ;
	int status;
	char last[2] = "&";
	int background = 0;
	if(strcmp(m_argv[m_argc-1],last)==0){
		m_argv[m_argc-1] = (char *)NULL;
		background = 1;
	}
	else{
		m_argv[m_argc] = (char*)NULL;
	}
	pid = fork();
	if(pid==0){
		if(execvp(m_argv[0],m_argv) < 0){
			perror("Execvp failed.");
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
int parseAndExec(char* ip){
	// defining some variable
	int i,len = strlen(ip),index=0;
	char *pch,str[1024],cmd[100];
	
	// copying original ip in str because strtok modifies string input
	bzero(str,sizeof(str));
	strcpy(str,ip);

	// first token --> it is command
	pch = strtok (str," ");
	if(pch == NULL){
		// this means user has pressed some whitespaces and gave the command(pressed enter) !
		return -1;
	}
	int m_argc = 0;
	char ** m_argv;
	m_argv = (char**)malloc(10*sizeof(char*));
	for(i=0;i<10;i++){
		m_argv[i] = (char*)malloc(100*sizeof(char));
	}
	strcpy(cmd,pch);
	strcpy(m_argv[m_argc++],pch);
	//this loops retrieves next tokes and stores them in m_argv array
	if(pch != NULL){
		while(pch!=NULL){
			pch = strtok (NULL," ");
			if(pch){
				strcpy(m_argv[m_argc++],pch);
			}
		}
	}

	// find out which command it is	
	int found = -1;
	int built_in_count = (sizeof(built_in)/sizeof(built_in[0]));
	for(i=0;i<built_in_count;i++){
		if(strcmp(cmd,built_in[i]) == 0){
			found = i;
		}
		
	}
	
	// call functions accordingly and pass m_argc and m_argv
	int status = 0;
	switch(found){
		case -1:
			//printf("other command\n");
			status = other_func(m_argc,m_argv);
			break;
		case 0:
			//printf("%s\n",built_in[0]);
			status = cd_func(m_argc-1,&m_argv[1]);
			break;
		case 1:
			//printf("%s\n",built_in[1]);
			status = pwd_func(m_argc-1,&m_argv[1]);
			break;
		case 2:
			//printf("%s\n",built_in[2]);
			status = mkdir_func(m_argc-1,&m_argv[1]);
			break;
		case 3:
			//printf("%s\n",built_in[3]);
			status = rmdir_func(m_argc-1,&m_argv[1]);
			break;
		case 4:
			//printf("%s\n",built_in[4]);
			status = ls_func(m_argc-1,&m_argv[1]);
			break;
		case 5:
			//printf("%s\n",built_in[5]);
			status = cp_func(m_argc-1,&m_argv[1]);
			break;
		case 6:
			//printf("%s\n",built_in[6]);
			status = exit_func(m_argc-1,&m_argv[1]);
			break;
		default:
			perror("command retrieving error.");
			status = -1;
			break;
	}

	return status;
}
void handle_signal(int signo){
	printf("\n%s>$ ",cwd);
	fflush(stdout);
}