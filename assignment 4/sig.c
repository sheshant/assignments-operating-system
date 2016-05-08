#include <stdio.h> 
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#define BUFFSIZE 1024
#define MSGSZ     128
#define ROCK		1
#define PAPER		2
#define SCISSOR		3

double val1 = 0;
double val2 = 0;
int pip1[2];
int pip2[2];
pid_t pides[3];
char buff[BUFFSIZE];
pid_t cpid,mypid, parpid;

void child1()
{
	srand((unsigned int)time(NULL));
	int r = 1+rand()%3;
	bzero(buff,BUFFSIZE);
	sprintf(buff,"%d",r);
	write(pip1[1],buff,BUFFSIZE);
	close(pip1[0]);
	sleep(1);
	return;
}
void child2()
{	
	srand((unsigned int)time(NULL));
	int r = 1+rand()%3;
	bzero(buff,BUFFSIZE);
	sprintf(buff,"%d",r);
	write(pip2[1],buff,BUFFSIZE);
	close(pip2[0]);
	sleep(1);
	return;
}
void exiting()
{
	exit(0);
}
char* str(int a)
{
	if(a == 1)
		return ((char*)"ROCK");
	if(a == 2)
		return ((char*)"PAPER");
	if(a == 3)
		return ((char*)"SCISSOR");
}
int main(int argc, char *argv[])
{
	srand((unsigned int)time(NULL));
	bzero(buff,BUFFSIZE);
	int i,status;
	int killr;
	pides[0] = getpid();
	if (pipe(pip1) == -1) 
    { 
        perror("pipe"); 
            exit(EXIT_FAILURE); 
    }
    if (pipe(pip2) == -1) 
    { 
        perror("pipe"); 
            exit(EXIT_FAILURE); 
    }
    if(pides[1]=fork())
    { //parent
    	if(pides[2]=fork())
    	{
    	} //parent
    	else 
    	{
    		signal(SIGUSR1,child2);
	    	signal(SIGUSR2,exiting);
	    	while(1)
	    	{
	    		sleep(1);
	    	}
    	}// child2 (D)
	}
	else
	{
	    signal(SIGUSR1,child1);
    	signal(SIGUSR2,exiting);
    	while(1)
    	{
    		sleep(1);
    	}
	}
    if(getpid() == pides[0])
    {
    	close(pip1[1]);
    	close(pip2[1]);
    	printf("The game begins with initial score \n");
    	printf("initial score of player1 = %lf\n", val1);
	    printf("initial score of player2 = %lf\n", val2);
	    printf("\n---------------------------------------------------------------------------------\n\n");
    	while(1)
    	{
	    	while(val1<10 && val2<10)
	    	{
	    		int p1=0,p2= 0;
	    		sleep(1);
	    		killr = kill(pides[1],SIGUSR1);
	    		sleep(1);
	    		killr = kill(pides[2],SIGUSR1);
	    		bzero(buff,BUFFSIZE);
	    		read(pip1[0],buff,BUFFSIZE);
		 		sscanf(buff,"%d",&p1);
		 		bzero(buff,BUFFSIZE);
	    		read(pip2[0],buff,BUFFSIZE);
		 		sscanf(buff,"%d",&p2);
		 		printf("player1 = %s\n", str(p1));
		 		printf("player2 = %s\n", str(p2));
		 		if(p1 == p2)
		 		{
		 			val1 +=0.5;
		 			val2 +=0.5;
		 		}
		 		else if((p1 == ROCK && p2 == PAPER) || (p1 == PAPER && p2 == SCISSOR) || (p1 == SCISSOR && p2 == ROCK))
		 		{
		 			val2 +=1;
		 		}
		 		else if((p2 == ROCK && p1 == PAPER) || (p2 == PAPER && p1 == SCISSOR) || (p2 == SCISSOR && p1 == ROCK))
		 		{
		 			val1 +=1;
		 		}
		 		else
		 		{
		 			printf("there is some error idiot\n");
		 		}
		 		printf("score of player1 = %lf\n", val1);
	    		printf("score of player2 = %lf\n", val2);
	    		printf("\n---------------------------------------------------------------------------------\n\n");
	    	}
	    	printf("end score of player1 = %lf\n", val1);
	    	printf("end score of player2 = %lf\n", val2);
	    	if(val1>=10 && val2>=10)
	    	{
	    		int r = rand();
	    		if(r%2 == 0)
	    		{
	    			printf("it gives head player 1 wins\n");
	    		}
	    		else
	    		{
	    			printf("it gives tail player 2 wins\n");
	    		}
	    	}
	    	else if(val1>=10 && val2<10)
	    	{
	    		printf("player 1 wins with score = %lf Congratulations \n", val1);
	    	}
	    	else if(val1<10 && val2>=10)
	    	{
	    		printf("player 2 wins with score = %lf Congratulations \n", val2);
	    	}
	    	else
	    	{
	    		printf("again there is some error idiot\n");
	    	}
	    	killr = kill(pides[1],SIGUSR2);
	    	killr = kill(pides[2],SIGUSR2);
	    	exit(0);
	    }
    }
    return 0;
}