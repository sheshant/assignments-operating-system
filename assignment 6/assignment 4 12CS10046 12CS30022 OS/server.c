#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <time.h>
#include <unistd.h>

#define MSGSZ    1280
#define BUFFSIZE 1024



void exiting()
{
	exit(0);
}


typedef struct mapping
{
	long msgpid;
	char* nameID;
	int deleted;

}mapping;

typedef struct msgbuf {
    long    mtype;
    char    mtext[MSGSZ];
} message_buf;


char* new = (char* )("NEW");
char* mesg = (char* )("MSG");
char* list = (char* )("LIST");

int main()
{
	struct tm *tmp;
	char buff[BUFFSIZE];
	size_t buf_length;
	key_t up = 10;
	key_t down = 20;
	int upid,downid;
	int msgflg = IPC_CREAT | 0666;
	int count = 0,i=0;
	int user_count = 0;
	int length = 0,j = 0;
	long recpid;  // to store receive pid
	time_t rectime; // to store receive time
	long sendpid;
	time_t sendtime;
	struct msqid_ds data;
	struct msqid_ds data_send;
	message_buf msg;
	message_buf msg_send;
	mapping map[100];
	if ((upid = msgget(up, msgflg )) < 0) {
        perror("msgget");
        exit(1);
    }
    if ((downid = msgget(down, msgflg )) < 0) {
        perror("msgget");
        exit(1);
    }
    char** chat;
    chat = (char** )malloc(100*sizeof(char));
    for(i = 0;i<100;i++)
    {
    	chat[i] = (char* )malloc(100*sizeof(char));
    }
    char* msgpre;
    char* id;
    char* new_msg;
    char* sendid;
    char* rtime;
    msgpre = (char* )malloc(100*sizeof(char));   	// this is to get the pre ie type of the message
    id = (char* )malloc(100*sizeof(char));			// this is to get the sender id in case of new client
    new_msg = (char* )malloc(1000*sizeof(char));	// this is to store the new message
    sendid = (char* )malloc(100*sizeof(char)); 		// this is to get the id where the message is to be sent
    rtime = (char* )malloc(100*sizeof(char)); 		// this is to get the time in string
 	
 	// the infinite loop begins
	while(1)
	{
		if(msgrcv(upid,&msg,MSGSZ,0,0)==-1) 	//Kernel to user memory space 
		{   
		    perror("msgrv failed\n");
		    exit(1);
		}  
		else
		{
			if(msgctl(upid,IPC_STAT,&data)==-1) 	//Kernel to user memory space 
			{   
			    perror("msgctl failed\n");
			    exit(1);
			} 
			else
			{
				printf("number of messages in receive message queue is %d\n",(int)data.msg_qnum );
				// first we need to find message parameters
				recpid = (long)data.msg_lspid;
				rectime = data.msg_rtime;
				struct tm *tmp = gmtime(&rectime);
				length = strlen(msg.mtext);
				i = 0;
				bzero(msgpre,100);
				while(msg.mtext[i] != 32)
				{
					msgpre[i] = msg.mtext[i];
					i++;
				}
				msgpre[i+1] = '\0';
				// we got the type
				// now we need to find the ID
				bzero(id,100);
				i = length - 1;
				while(msg.mtext[i] != 32)
				{
					i--;
				}
				i = i+1;
				int j = 0;
				while(i<length)
				{
					id[j] = msg.mtext[i];
					i++;
					j++;
				}
				id[j+1] = '\0';
				// so we got the id also

				// case 1 if type is new
				// then we have to send message to all about this client id after adding it 
				if(strcmp(new,msgpre) == 0)
				{
					// display from where you got message
					printf("message received\n%s\nreceived pid = %ld\nclientID = %s\nreceivedtime = %d:%d:%d\n"
						,msg.mtext,recpid,id,tmp->tm_hour,tmp->tm_min,tmp->tm_sec );
					printf("new client id is %s\n",id );
					// create new map for that
					map[count].nameID = (char* )malloc(100*sizeof(char));
					strcpy(map[count].nameID,id);
					map[count].deleted = 0;
					map[count].msgpid = recpid;
					count++;
					user_count++;
					bzero(new_msg,1000);
					// add the tag ie type list to the message to be sent
					strcat(new_msg,list);
					// add all online clients
					for(i = 0;i<count;i++)
					{
						if(map[i].deleted == 0)
						{
							strcat(new_msg,(char *)(" "));
							strcat(new_msg,map[i].nameID);
						}
					}
					printf("%s\n", new_msg);
					// then send message to all online clients
					for(i = 0;i<count;i++)
					{
						msg_send.mtype = (long)map[i].msgpid;
						strcpy(msg_send.mtext,new_msg);
						buf_length = strlen(msg_send.mtext) + 1 ;
						printf("message going to be send to %ld\n", msg_send.mtype);
						if(msgsnd(downid,&msg_send,buf_length,0) == -1) //User to Kernel memory space 
						{
							printf("error\n");
							exit(1);
						}
						else
						{
							printf("message is\n%s\n send to %s\n",msg_send.mtext,map[i].nameID);
						}
					}
				}
				// case 2 in case if we receive a chat message and we need to send it to the 
				// specified user  
				else if(strcmp(mesg,msgpre) == 0)
				{
					printf("message received\n%s\nreceived pid = %ld\nclientID = %s\nreceivedtime = %d:%d:%d\n"
						,msg.mtext,recpid,id,tmp->tm_hour,tmp->tm_min,tmp->tm_sec );
					// first we need to get the chat id of the receiver of this message
					int index;
					for(i = 0;i<length;i++)
					{
						if(msg.mtext[i] == 32 && i!=length-1)
						{
							index = i+1;
						}
					}
					// printf("index = %d\n",index );
					bzero(sendid,100);
					j = 0;
					int new_index = index;
					while(index<length)
					{
						sendid[j] = msg.mtext[index];
						index++;
						j++;
					}
					sendid[j+1] = '\0';
					// we got it in sendid
					// now we need to get the pid of the receiver of this message
					int map_index = -1;
					for(i = 0;i<count;i++)
					{
						if(strcmp(sendid,map[i].nameID) == 0)
						{
							map_index =i;
						}
					}
					// we got the index
					// now we will access the data regarding this message

					if(map_index != -1)
					{
						// first we need to frame the message string
						bzero(new_msg,1000);
						for(i = 0;i<new_index;i++)
						{
							new_msg[i] = msg.mtext[i];
						}
						new_msg[i+1] = '\0';
						// now we need to add the time
						bzero(rtime,100);
						sprintf(rtime , "%d:%d:%d",tmp->tm_hour,tmp->tm_min,tmp->tm_sec );
						strcat(new_msg,(char *)(" "));
						strcat(new_msg,rtime);
						strcat(new_msg,(char *)(" "));
						strcat(new_msg,id);
						// after framing we just simply need to send
						msg_send.mtype = (long)map[map_index].msgpid;
						strcpy(msg_send.mtext,new_msg);
						buf_length = strlen(msg_send.mtext) + 1 ;
						printf("message send to %ld\n", msg_send.mtype);
						if(msgsnd(downid,&msg_send,buf_length,0) == -1) // User to Kernel memory space 
						{
							printf("error\n");
							exit(1);
						}
						else
						{
							printf("%s\nto \n%s\n",msg_send.mtext,sendid );
							if(msgctl(downid,IPC_STAT,&data_send)==-1) 	//Kernel to user memory space 
							{   
							    perror("msgctl failed\n");
							    exit(1);
							}
							else
							{
								printf("number of messages in send message queue is %d\n",(int)data_send.msg_qnum );
							} 
						}
					}
					else
					{
						printf("there is some error idiot\n");
					}
				}
			}
		}
	}
	return 0;
}