#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include  <fcntl.h>

#define REQUEST 100
#define PIVOT 200
#define LARGE 300
#define SMALL 400
#define READY 500
#define CHECK 600

#define BUFFSIZE 1024

/****************************

*	0 - parent to child 1
*	1 - parent to child 2
*	2 - parent to child 3
*	3 - parent to child 4
*	4 - parent to child 5


*	5 - child 1 to parent
*	6 - child 2 to parent
*	7 - child 3 to parent
*	8 - child 4 to parent
*	9 - child 5 to parent

******************************/


int main(){
	//buffer string
	char buff[BUFFSIZE];
	bzero(buff,BUFFSIZE);

	//pipe
	int fd[10][2],i;
	for(i=0;i<10;i++)
		pipe(fd[i]); 
	srand((unsigned int)time(NULL));
	//compile child.c
	system("gcc -o child.out child.c");

	//fork five child
	pid_t child_id [5];
	for(i=0;i<5;i++){
		child_id[i] = fork();
		if(child_id[i]==0){

			close(fd[i][1]);
			close(fd[i+5][0]);

			char **m_argv;
			m_argv = (char**)malloc(100*sizeof(char*));
			int j;
			for(j=0;j<100;j++){
				m_argv[j] = (char*)malloc(100*sizeof(char));
			}

			sprintf(buff,"%d\n",fd[i][0]);
			strcpy(m_argv[0],buff);
			sprintf(buff,"%d\n",fd[i+5][1]);
			strcpy(m_argv[1],buff);
			m_argv[2] = (char*)NULL;

			execv("./child.out",m_argv);

		}
	}
	for(i=0;i<5;i++){

		// close useless read-write ends
		close(fd[i][0]);
		close(fd[i+5][1]);

		//send child his id
		sprintf(buff,"%d\n",i+1);
		write(fd[i][1],buff,BUFFSIZE);
 			
	}
	//checking if all child are ready
	int count=0;
	for(i=0;i<5;i++){
		bzero(buff,BUFFSIZE);
		read(fd[i+5][0],buff,BUFFSIZE);
		int temp;
		sscanf(buff,"%d",&temp);
		if(temp == READY){
			count++;
		}
	}

	if(count==5){
		printf("--- Parent READY\n");
	}
	//feed algo inside
	int med_found = 0;
	int m =0;int pivot = 0;
	int n = 25;int k = n/2,r,num = 0;
	int b[5];


	while(med_found == 0)
	{
		int dex_flag = 0;
		while(dex_flag == 0)
		{
			bzero(buff,BUFFSIZE);
			r = rand() %5;
			sprintf(buff,"%d\n",REQUEST);
			write(fd[r][1],buff,BUFFSIZE);
			printf("--- Parent sends REQUEST to Child %d\n", r+1);
			bzero(buff,BUFFSIZE);
			read(fd[r+5][0],buff,BUFFSIZE);
			int temp;
			sscanf(buff,"%d",&temp);
			if(temp != -1)
			{
				dex_flag = 1;
				pivot = temp;
			}
			else
			{
				printf("--- Parent received -1 , sends request again\n");
				dex_flag = 0;
			}
		}
	 	printf("--- Parent broadcasts pivot %d to all children\n", pivot);
	 	for(i = 0;i<5;i++)
	 	{
	 		bzero(buff,BUFFSIZE);
	 		sprintf(buff,"%d\n",PIVOT);
	 		write(fd[i][1],buff,BUFFSIZE);
	 		bzero(buff,BUFFSIZE);
	 		sprintf(buff,"%d\n",pivot);
	 		write(fd[i][1],buff,BUFFSIZE);
	 	}
	 	num = 0;
	 	for(i = 0;i<5;i++)
	 	{
	 		bzero(buff,BUFFSIZE);
	 		read(fd[i+5][0],buff,BUFFSIZE);
	 		int ct;
	 		sscanf(buff,"%d",&ct);
	 		num += ct;
	 	}
	 	m = num;
	 	// printf("m = %d  k = %d\n",m,k );
	 	if(m>k)
	 	{
	 		for(i = 0;i<5;i++)
	 		{
	 			bzero(buff,BUFFSIZE);
	 			sprintf(buff,"%d\n",SMALL);
	 			write(fd[i][1],buff,BUFFSIZE);
	 		}
	 	}
	 	else if(m<k)
	 	{
	 		for(i = 0;i<5;i++)
	 		{
	 			bzero(buff,BUFFSIZE);
	 			sprintf(buff,"%d\n",LARGE);
	 			write(fd[i][1],buff,BUFFSIZE);
	 		}
	 		k = k-m;
	 	}
	 	else
	 	{
	 		med_found = 1;
	 		num = 0;
	 		for(i = 0;i<5;i++)
	 		{
	 			bzero(buff,BUFFSIZE);
	 			sprintf(buff,"%d\n",CHECK);
	 			write(fd[i][1],buff,BUFFSIZE);
	 			bzero(buff,BUFFSIZE);
	 			sprintf(buff,"%d\n",pivot);
	 			write(fd[i][1],buff,BUFFSIZE);
	 		}
	 		for(i = 0;i<5;i++)
	 		{
	 			bzero(buff,BUFFSIZE);
		 		read(fd[i+5][0],buff,BUFFSIZE);
		 		int ct;
		 		sscanf(buff,"%d",&ct);
		 		b[i] = ct;
		 		num += ct;
	 		}
	 		printf("--- Parent: m = %d",b[0]);
	 		for(i = 1;i<5;i++)
	 		{
	 			printf(" + %d",b[i] );
	 		}
	 		printf(" = %d . ",num);
	 		printf(" %d = %d/2\n",num,25);
	 	}
	 }


	// you got median
	//killer loop
	if(med_found==1){
		printf(" median is %d\n", pivot);
		printf("--- Parent sends kill signals to all children\n");
		sleep(1);
		for(i=0;i<5;i++){
			//kill child
			sleep(1);		
			kill(child_id[i],SIGUSR1);
			wait(NULL);
		}
	}
	return 0;
}

