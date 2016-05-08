#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define REQUEST 100
#define PIVOT 200
#define LARGE 300
#define SMALL 400
#define READY 500
#define CHECK 600

#define BUFFSIZE 1024

int my_id;

void childSigHandler (int);

int main(int argc, char const *argv[])
{

	//register signal handler
	signal(SIGUSR1, childSigHandler);
	// get its read and write end
	int read_end,write_end;
	sscanf(argv[0],"%d",&read_end);
	sscanf(argv[1],"%d",&write_end);

	// buffer string
	char buff[BUFFSIZE];
	bzero(buff,BUFFSIZE);

	// file pointer
	char name[] = "data_0.txt";
	FILE *fp;

	//waits for his parent to send his id
	read(read_end, buff, BUFFSIZE);

	// open corresponding file
	char id  = buff[0];
	my_id = id - '0';
	name[5] = id;
	fp = fopen(name,"r");

	//store num in arr
	int num_arr[5],actual[5],i;
	for(i=0;i<5;i++){
		fscanf(fp,"%d",&num_arr[i]);
		actual[i] = num_arr[i];
	}

	int arr_size = 5;
	// for(i=0;i<arr_size;i++)
	// 			{
	// 				printf("%d ",num_arr[i]);
	// 			}
	// 			printf("%d\n", my_id);

	//send ready signal to parent
	bzero(buff,BUFFSIZE);
	sprintf(buff,"%d\t",READY);
	printf("--- Child %d sends READY\n", my_id);
	write(write_end,buff,BUFFSIZE);
	int b[5],cnt;

	while(1){
		//read code
		read(read_end, buff, BUFFSIZE);
		int temp,pivot;
		sscanf(buff,"%d",&temp);
		
		//switch accordingly
		switch(temp){

			case REQUEST:
				//printf("received REQUEST\n");
				if(arr_size == 0){
					bzero(buff,BUFFSIZE);
					sprintf(buff,"-1");
					printf("--- Child %d sends -1 to parent\n",my_id);
					write(write_end,buff,BUFFSIZE);
				}
				else{
					int key  = (unsigned int)rand()%arr_size;
					bzero(buff,BUFFSIZE);
					sprintf(buff,"%d\t",num_arr[key]);
					printf("--- Child %d sends %d to parent\n",my_id,num_arr[key]);
					write(write_end,buff,BUFFSIZE);
				}
				break;

			case SMALL:
				//printf("received SMALL\n");
				
				for(i=0;i<arr_size;i++){
					b[i] = num_arr[i];
				}
				cnt = 0;
				for (i=0;i<arr_size;i++){
					if(b[i]>=pivot){
						num_arr[cnt++] = b[i];
					}
				}
				arr_size = cnt;
				// printf("After drop size = %d\n",arr_size);
				// for(i=0;i<arr_size;i++)
				// {
				// 	printf("%d ",num_arr[i]);
				// }
				// printf("%d\n", my_id);
				break;

			case LARGE:
				//printf("received LARGE\n");
				
				for(i=0;i<arr_size;i++){
					b[i] = num_arr[i];
				}
				cnt = 0;
				for (i=0;i<arr_size;i++){
					if(b[i]<=pivot){
						num_arr[cnt++] = b[i];
					}
				}
				arr_size = cnt;
				// printf("After drop size = %d\n",arr_size);
				// for(i=0;i<arr_size;i++)
				// {
				// 	printf("%d ",num_arr[i]);
				// }
				// printf("%d\n", my_id);
				break;

			case PIVOT:
				//printf("received PIVOT\n");
				//read pivot num
				read(read_end, buff, BUFFSIZE);
				sscanf(buff,"%d",&pivot);
				cnt = 0;
				for(i=0;i<arr_size;i++){
					if(num_arr[i]>pivot){
						cnt++;
					}
				}
				bzero(buff,BUFFSIZE);
				sprintf(buff,"%d\t",cnt);
				printf("--- Child %d receives pivot and replies %d\n",my_id,cnt);
				write(write_end,buff,BUFFSIZE);
				break;

			case CHECK:
				read(read_end, buff, BUFFSIZE);
				sscanf(buff,"%d",&pivot);
				cnt = 0;
				for(i=0;i<5;i++){
					if(actual[i]>pivot){
						cnt++;
					}
				}
				bzero(buff,BUFFSIZE);
				sprintf(buff,"%d\t",cnt);
				write(write_end,buff,BUFFSIZE);
				break;

			default:
				printf("Invalid code received\n");
				break;
		}

	}
	return 0;
}

void childSigHandler (int sig){
	if(sig==SIGUSR1){
		//terminate process
		printf("--- Child %d terminates\n", my_id);

	}
	else{
		perror("Wrong signal.");
	}
	exit(0);

}