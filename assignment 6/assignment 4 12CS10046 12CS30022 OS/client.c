#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define MSGSZ     1280

void exiting();
void display_chat();
void update(char * user);

typedef struct  msgbuf {
    long    mtype;
    char    mtext[MSGSZ];
} message_buf;

key_t up = 10;
key_t down = 20;
int upid,downid;
int msgflg = IPC_CREAT | 0666;
char* new = (char* )("NEW");
char* mesg = (char* )("MSG");
char* list = (char* )("LIST");
char** chat;
int user_count = 0;

int main()
{
	printf("pid is %d\n",(int)getpid() );
	message_buf msg;
	message_buf msg_rec;
    size_t buf_length;
    int length,k;
    int chat_id;

	if ((upid = msgget(up, msgflg )) < 0) {
        perror("msgget");
        exit(1);
    }
    if ((downid = msgget(down, msgflg )) < 0) {
        perror("msgget");
        exit(1);
    }
    int i=0;
    
    chat = (char** )malloc(100* sizeof(char*));
    for(i = 0;i<100;i++)
    {
    	chat[i] = (char* )malloc(100* sizeof(char));
    }
    char * name;
    name = (char*)malloc(100* sizeof(char));
    char * text;
    text = (char*)malloc(100* sizeof(char));
    // char  text2[100];
    // text2 = (char*)malloc(100* sizeof(char));
    char * msg_send;
    msg_send = (char*)malloc(100* sizeof(char));
    char * msg_receive;
    msg_receive = (char*)malloc(100* sizeof(char));
    char * msgpre;
    msgpre = (char*)malloc(100* sizeof(char));
    printf("Enter chat id (remember chat id is without spaces) \n");
  	scanf("%s",name);
  	printf("Your chat id is %s\n",name);
  	strcpy(text,(char* )("NEW "));
  	strcat(text,name);
  	strcpy(msg.mtext,text);
  	msg.mtype=1;								//User memory Space 
	buf_length = strlen(msg.mtext) + 1 ;
	if(msgsnd(upid,&msg,buf_length,0)==-1) 		//User to Kernel memory space 
	{
		printf("error\n");
		exit(1);
	}
	else
	{
		if(msgrcv(downid,&msg_rec,MSGSZ,(long)getpid(),0)==-1) 	//Kernel to user memory space 
		{   
		    perror("msgrv failed\n");
		    exit(1);
		} 
		else
		{
			printf("%s\n", msg_rec.mtext);
			update(msg_rec.mtext);
		}
	}
	while(1)
	{
		// we enter the loop first we need to display the chat
		display_chat();
		scanf("%d",&chat_id);
		if(chat_id <= user_count)
		{
			// if chat id is valid 
			// there is no proper error handling for invalid chat id
			printf("Enter your message\n");
			bzero(text,100);
			scanf("%s",text);
			// we got hte message from the user now we frame the message in proper order
			strcat(msg_send,mesg);
			strcat(msg_send,(char *)(" "));
			strcat(msg_send,text);
			strcat(msg_send,(char *)(" "));
			strcat(msg_send,chat[chat_id]);
			printf("%s\n",msg_send );  // the message which we will send
			strcpy(msg.mtext,msg_send);
			bzero(msg_send,100);
			msg.mtype = 1;// since we need to send it to the server , we do not need specified mtype
			buf_length = strlen(msg.mtext) + 1 ;
			if(msgsnd(upid,&msg,buf_length,0)==-1) 		// User to Kernel memory space 
			{
				printf("error\n");
				exit(1);
			}
			// after sending the message we will receive the message from down queue
			// which is specified to our process
			if(msgrcv(downid,&msg_rec,MSGSZ,(long)getpid(),0)==-1) 	// Kernel to user memory space 
			{   
			    perror("msgrv failed\n");
			    exit(1);
			} 
			else
			{
				length = strlen(msg_rec.mtext);
				i = 0;
				bzero(msgpre,100);
				while(msg_rec.mtext[i] != 32 && i<length)
				{
					msgpre[i] = msg_rec.mtext[i];
					i++;
				}
				msgpre[i+1] = '\0';
				// we got the type
				// 2 cases first if it is list or msg_send
				// case 1 list
				if(strcmp(list,msgpre) == 0)
				{
					// in this case we need to update the chat list
					update(msg_rec.mtext);
				}
				// case 2 if it not a list
				else if(strcmp(mesg,msgpre) == 0)
				{
					// here we need to display the chat message with user and time
					printf("\n%s\n\n", msg_rec.mtext);
				}
				// there is no other case
				else
				{
					printf("I guess there is something wrong\n");
				}
			}
		}
		else
		{
			printf("chat id invalid\n");
		}
	}
	return 0;
}
void update(char user[MSGSZ])
{
	user_count = 0;
	int len = strlen(user);
	int i,j = 0,k = 0;
	char * myuser;
    myuser = (char*)malloc(100* sizeof(char));
	for(i = 5;i<len;i++)
	{
		if(user[i] == ' ')
		{
			myuser[j] = '\0';
			strcpy(chat[k],myuser);
			j=0;
			bzero(myuser,100);
			k++;
		}
		else
		{
			myuser[j] = user[i];
			j++;
		}
	}
	myuser[j] = '\0';
	strcpy(chat[k],myuser);
	user_count = k;
}
void display_chat()
{
	int i;
	printf("available friends with their index number\n");
	for(i = 0;i<=user_count;i++)
	{
		printf("%d\t%s\n",i,chat[i]);
	}
	printf("choose the index number from %d to %d\n",0,user_count);
}
void exiting()
{
	exit(0);
}