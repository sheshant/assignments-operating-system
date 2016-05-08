#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <fcntl.h>
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
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <err.h>
#include <errno.h>
#include <langinfo.h>
#include <limits.h>
#include <locale.h>

#define MSGSZ    1280
#define SIZE    100



// message buffer
typedef struct msgbuf {
    long    mtype;
    char    mtext[MSGSZ];
} message_buf;

// global variables
int msgid,sem1,sem2,shm1,shm2; // related to ipc
key_t sem1key = 20;
key_t sem2key = 40;
key_t shm1key = 60;
key_t shm2key = 80;
key_t msgkey = 100;

int msgflg = IPC_CREAT | 0666;
char ** list;int listsize;// this is to read the argument values
char ** pid_list;// this is to read the pid from shared memory
int pid_list_size = 0;  /// and it's size
char* buff,*name,*temp;
char *ptrpid ,*ptrmsg;
char *ptrmsg_copy,* sender_pid;
message_buf msg_send;// message buffer to be send
int send_pid;// to store pid where to send

FILE *fp,*fputmp;

int i,j,k;

struct sembuf sop; // for semaphores
struct utmp u;// this is for UTMP file

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
	sop.sem_op = 0;
	// 0 indicates that it will proceed only if the value of the semaphore is 0 else block
	sop.sem_flg = 0;
	semop(mutex, &sop, 1);*/
	return;
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
void setvalue(int mutex,int n)
{
	if ((semctl(mutex, 0, SETVAL, n)) < 0) 
    {
        perror("semctl error");
        exit(EXIT_FAILURE);
    }
}
void terminate()
{
	shmdt((char *)ptrpid);
	shmdt((char *)ptrmsg);
	free(ptrmsg_copy);
	free(list);
	free(buff);
	free(name);
	free(temp);
	free(sender_pid);
	fclose(fp);
	fclose(fputmp);
	semctl(sem1, 0, IPC_RMID, 0);
	semctl(sem2, 0, IPC_RMID, 0);
	shmctl(shm1, IPC_RMID, 0);
	shmctl(shm2, IPC_RMID, 0);
	msgctl(msgid,IPC_RMID,NULL);
	if((remove("ser.txt")) == -1)
	{
		perror("file isn't deleted");
		exit(EXIT_FAILURE);
	}
	exit(0);
}


int main(int argc, char *argv[])
{
	int count = argc;
	const char *file;
    file = _PATH_UTMP;
	list = (char**)malloc(SIZE*sizeof(char*));
	for(i = 0;i<SIZE;i++)
	{
		list[i] = (char*)malloc(SIZE*sizeof(char));
	}
	pid_list = (char**)malloc(SIZE*sizeof(char*));
	for(i = 0;i<SIZE;i++)
	{
		pid_list[i] = (char*)malloc(SIZE*sizeof(char));
	}
	buff = (char*)malloc(SIZE*sizeof(char));
    name = (char*)malloc(SIZE*sizeof(char));
    temp = (char*)malloc(SIZE*sizeof(char));
    ptrmsg_copy = (char*)malloc(SIZE*sizeof(char));
    sender_pid = (char*)malloc(SIZE*sizeof(char));
    bzero(sender_pid,SIZE);
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
    if ((semctl(sem1, 0, SETVAL, 1)) < 0) 
    {
        perror("semctl");
        exit(EXIT_FAILURE);
    }
    if ((sem2 = semget(sem2key,1, msgflg )) < 0) 
    {
        perror("semget");
        exit(EXIT_FAILURE);
    }
    if ((semctl(sem2, 0, SETVAL, 0)) < 0) 
    {
        perror("semctl");
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
    bzero(ptrpid,500);
    bzero(ptrmsg,5000);
    printf("--- Initialization complete\n");
    // done 
    // then read all names 
    j = 0;
	while(argc>1)
	{
		strcpy(list[j],argv[--argc]);
		j++;
	}
	listsize = j;

	// open a file in excl to ensure that only one server is running
	fp = fopen("ser.txt","wx");
	if(fp == NULL)
	{
		perror("server already running");
		exit(EXIT_FAILURE);
	}
	bzero(buff,SIZE);
	sprintf(buff,"%d",getpid());
	fwrite(buff,SIZE,1,fp);

	// here comes the UTMP part
	// we need to open the file in the linux for the various terminals opened 
	if ((fputmp = fopen(file, "r")) == NULL)
    {
    	err(1, "%s", file);
    }
    while (fread(&u, sizeof(u), 1, fputmp) == 1)
	{
		if (u.ut_name != NULL) 
		{
			if(getpwnam(u.ut_name) != NULL) // means the file is not arbit 
			{
				// printf("%s\n",u.ut_line );
				for(i = 0;i<listsize;i++)
				{
					if(((strcmp(u.ut_name,list[i])) == 0) && (count > 1) && ((strcmp(u.ut_line,":0")) != 0)) 
						// means it points to real user who should be in conference
					{
						// so we need to send the commencement notification to the corresponding user
						printf("--Sending commence notification to %s\n",u.ut_line );
						sprintf(name, "./commence > /dev/%s", u.ut_line);
						system(name);
						count--;
						break;
					}
				}
			}
		}
	}
	// from here the infinite loop begins to broadcast the message to all 
	// it waits until the value of the sem2 = 2 which indicates client has written the message
	while(1)
	{
		wait_for(sem2,2);
		// now read message
		strcpy(ptrmsg_copy,ptrmsg);
		printf("%s\n",ptrmsg_copy );
		setvalue(sem2,0);
		// case 1 if the message is '.' then we need to simply ignore it
		if((strcmp(ptrmsg_copy,".")) == 0)
		{

		} 
		// case 2 if the message is '*' then we need to terminate server as there is no client 
		else if((strcmp(ptrmsg_copy,"*")) == 0)
		{
			terminate();
		}
		// case 3 now we need to broadcast the message to all the clients available except 
		// the one who send the message
		else
		{
			down(sem1);
			read_pid();
			up(sem1);
			strcpy(msg_send.mtext,ptrmsg_copy);
			int l = strlen(ptrmsg_copy)+1;
			k = 0,j = 0;
			while(ptrmsg_copy[j] != '/')
			{
				j++;
			}
			j++;
			while(ptrmsg_copy[j] != ':')
			{
				sender_pid[k] = ptrmsg_copy[j];
				j++;
				k++;
			}
			sender_pid[k] = '\0';
			for(i = 0;i<pid_list_size;i++)
			{
				if((strcmp(pid_list[i],sender_pid)) != 0)
				{
					// then send message as it is different user 
					sscanf(pid_list[i],"%d",&send_pid);
					printf("%d  %d  %d\n%s\n", msgid,l,send_pid,msg_send.mtext);
					msg_send.mtype = (long)send_pid;
					if(msgsnd(msgid,&msg_send,l,0)==-1) 		//User to Kernel memory space 
					{
						printf("error in msg_send\n");
						exit(1);
					}
					else
					{
						printf("sending message to %d\n",send_pid );
					}
				}
			}
		}
	}
	// deleting all the ipc
	
	return 0;
}