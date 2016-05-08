#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <utmp.h>
#include <pwd.h>
#include <fcntl.h>

#define MSGSZ    1280
#define SIZE    100



// message buffer
typedef struct msgbuf {
    long    mtype;
    char    mtext[MSGSZ];
} message_buf;

// global variables
char * login_name;
int msgid,sem1,sem2,shm1,shm2; // related to ipc
key_t sem1key = 20;
key_t sem2key = 40;
key_t shm1key = 60;
key_t shm2key = 80;
key_t msgkey = 100;

int msgflg = IPC_CREAT | 0666;
char* buff,*name,*temp;
char ** pid_list;
int pid_list_size = 0;
char *ptrpid, *ptrmsg;
pid_t pid,mypid;
char * my_pid, * main_msg;
message_buf msg_rec;

int i,j,k;

struct sembuf sop; // for semaphores

void exiting()
{
	shmdt((char *)ptrpid);
	shmdt((char *)ptrmsg);
	exit(0);
}
void down(int mutex)
{
	sop.sem_num = 0;
	sop.sem_op = -1;
	sop.sem_flg = 0;
	semop(mutex, &sop, 1);
}
void up(int mutex)
{
	sop.sem_num = 0;
	sop.sem_op = 1;
	sop.sem_flg = 0;
	semop(mutex, &sop, 1);
}
void wait_for(int mutex,int n)
{
	while((semctl(mutex,0,GETVAL,0)) != n);
	/*sop.sem_num = 0;
	sop.sem_op = 0;// 0 indicates that it will proceed only if the value of the semaphore is 0 else block
	sop.sem_flg = 0;
	semop(mutex, &sop, 1);*/
	return;
}
// function to set the value of semaphore
void setvalue(int mutex,int n)
{
	if ((semctl(mutex, 0, SETVAL, n)) < 0) 
    {
        perror("semctl error");
        exit(EXIT_FAILURE);
    }
}
void read_pid()
{
	int length = strlen(ptrpid);
	k = 0;pid_list_size = 0;
	for(i = 0;i<length;i++)
	{
		if(ptrpid[i] == ' ')
			// one pid is over to read now store it and read another pid
		{
			temp[k] = '\0';
			k = 0;
			strcpy(pid_list[pid_list_size],temp);
			pid_list_size++;
		}
		else
		{
			temp[k] = ptrpid[i];
			k++;
		}
	}
}
void send_msg()
{
	printf("\n--- Enter your message:\n");
	scanf(" %s",name);
	printf("You have entered %s\n",name );
	// now frame the message
	bzero(main_msg,SIZE);
	strcat(main_msg,login_name);
	strcat(main_msg,"/");
	strcat(main_msg,my_pid);
	strcat(main_msg,":");
	strcat(main_msg,name);
	// framed now we need to send it

	// it will only execute when the value of sem2 = 0 else wait for it to become 0
	// sem2 = 0 indicates server exits its critical section akfter reading message from shared memeory
	wait_for(sem2,0);
	// ok so it is zero now we need to set the value to 1 so that other client 
	// processes doesn't enter here and also the server
	setvalue(sem2,1);
	if((strcmp(name,"bye")) == 0)
		// bye indicates client is leaving so no need to write this  
	{
		down(sem1);
		read_pid();
		if(pid_list_size == 1)
		{
			// if it is the last user then we need to set the mssage to * indicating that the server has to exit
			strcpy(ptrmsg,"*");
		}
		else
		{
			strcpy(ptrmsg,main_msg);
		}
		// here the changes in the pid list in the shared memory is done which includes removing own pid
		bzero(ptrpid,500);
		for(i = 0;i<pid_list_size;i++)
		{
			if(strcmp(pid_list[i],my_pid) != 0)
			{
				// it means it is another pid then add it to the shared memeory of pid
				strcat(ptrpid,pid_list[i]);
				strcat(ptrpid,(char *)(" "));
			}
		}
		// strcpy(ptrmsg,"user left");
		setvalue(sem2,2);
		up(sem1);
		// and after sending we need to exit both child and parent
		kill(pid,SIGUSR1);
		exiting();
	}
	else if((strcmp(name,".")) == 0)
	{
		down(sem1);
		strcpy(ptrmsg,(char *)("."));
		setvalue(sem2,2);
		up(sem1);
	}
	else // then we need to simply copy it to the shared memory
	{
		down(sem1);
		strcpy(ptrmsg,main_msg);
		setvalue(sem2,2);
		up(sem1);
	}
	// we are done with sending the message now we need to set the value of sem1 to 2 such that
	// server process can access the message 
	// return;
}

int main(int argc, char *argv[])
{
	struct stat fileInfo;
	mypid = getpid();
	printf("Your login pid %d\n",mypid );
	login_name = getlogin();
	printf("Your login name %s\n",login_name );
	buff = (char*)malloc(SIZE*sizeof(char));
    name = (char*)malloc(SIZE*sizeof(char));
    main_msg = (char*)malloc(SIZE*sizeof(char));
    temp = (char*)malloc(SIZE*sizeof(char));
    my_pid = (char*)malloc(SIZE*sizeof(char));
    pid_list = (char**)malloc(SIZE*sizeof(char*));
	for(i = 0;i<SIZE;i++)
	{
		pid_list[i] = (char*)malloc(SIZE*sizeof(char));
	}
    bzero(my_pid,SIZE);
   	sprintf(my_pid,"%d",(int)mypid);
   	strcpy(main_msg,".");
	// checking whether the file exist or not
    if((stat("ser.txt", &fileInfo)) == -1)
    {
    	// it means file doesn't exist then that means server isn't ready so exit
    	perror("file doesn't exist");
        exit(EXIT_FAILURE);
    }
    // else server is ready so proceed
    // start with ipc resources
    // now set up the ipc for communication
	if ((msgid = msgget(msgkey, msgflg )) < 0) 
	{
        perror("msgget");
        exit(EXIT_FAILURE);
    }
    if ((sem1 = semget(sem1key,1, msgflg )) < 0) 
    {
        perror("semget");
        exit(EXIT_FAILURE);
    }
    
    if ((sem2 = semget(sem2key,1, msgflg )) < 0) 
    {
        perror("semget");
        exit(EXIT_FAILURE);
    }
    if ((shm1 = shmget(shm1key,500, msgflg )) < 0) 
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    if ((shm2 = shmget(shm2key,5000, msgflg )) < 0) 
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    ptrpid=shmat(shm1,NULL,0);
    ptrmsg=shmat(shm2,NULL,0);
    printf("---Initialization complete\n");
    // done 
    // now we need to fork two processes
    if((pid = fork()) == 0)
    	// this is the child process for receiving signal
    {
    	signal(SIGUSR1,exiting);
    	signal(SIGINT,SIG_IGN);//this isvery important to handle and ignore crtl+c  
    	down(sem1);
    	// critical region begins here 
    	// here we need to update the pid table in the shared memory by adding our own pid
    	strcat(ptrpid,my_pid);
    	strcat(ptrpid,(char *)(" "));
    	printf("users loged in \n");
    	printf("%s\n", ptrpid);
    	up(sem1);
    	// the infinite iterator begins here 
    	// it will receive message in each iteration from the message queue
    	while(1)
    	{
    		if(msgrcv(msgid,&msg_rec,MSGSZ,(long)mypid,0)==-1) 	//Kernel to user memory space 
			{   
			    perror("msgrcv failed\n");
			    exit(1);
			} 
			else
			{
				printf("--- Received message:'%s'\n", msg_rec.mtext);
			}
    	}
    }
    else
    	// here is the parent process for sending signal
    {
    	while(1)// wait for crtl + c in an infinite loop
    	{
			signal(SIGINT,send_msg);	
    	}
    }
	return 0;
}