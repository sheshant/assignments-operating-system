#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <semaphore.h>

int main()
{
	int chairs,custom;
	printf("total number of customers = ");
	scanf("%d",&custom);
	printf("total number of chairs = ");
	scanf("%d",&chairs);
	struct sembuf sop;
	int status,i,waiting = 0,j = custom;
	int n = 5;
	int mutex,customer,barber,count;
	key_t key=20;
	key_t key2=40;
	key_t key3=50;
	key_t key4=60;
	mutex = semget(key,1,0666|IPC_CREAT);
	barber = semget(key2,1,0666|IPC_CREAT);
	customer = semget(key3,1,0666|IPC_CREAT);
	count = semget(key4,1,0666|IPC_CREAT);
	semctl(mutex, 0, SETVAL, 1);
	semctl(customer, 0, SETVAL, chairs);// 0 here is subsem id
	semctl(barber, 0, SETVAL, 0);
	semctl(count, 0, SETVAL, custom);
	pid_t pid,mpid;
	mpid = getpid();
	for(i = 0;i<custom;i++)
	{
		if((pid = fork()) == 0)
		{
			sleep(i+1);
			// first check if there is no place
			if(semctl(customer,0, GETVAL,0) <= 0)
			// then he has to leave	
			{
				printf("customer %d leaves as there is no chair\n and waiting = %d\n",i,chairs-semctl(customer,0, GETVAL,0));
				sop.sem_num = 0;
				sop.sem_op = -1;
				sop.sem_flg = 0;
				semop(count, &sop, 1);
				exit(0);
			}
			else
			{
				//down customer
				sop.sem_num = 0;
				sop.sem_op = -1;
				sop.sem_flg = 0;
				semop(customer, &sop, 1);
				printf("customer %d occupy the chair %d\n",i,semctl(customer,0, GETVAL,0));
				// waiting++;
				// printf("%d\n",waiting);

				// down mutex
				sop.sem_num = 0;
				sop.sem_op = -1;
				sop.sem_flg = 0;
				semop(mutex, &sop, 1);
				printf("down mutex : s for mutex = %d\n",semctl(mutex,0, GETVAL,0));

				// up barber
				sop.sem_num=0;
				sop.sem_op=1;
				sop.sem_flg=0;
				semop(barber,&sop,1);
				printf("up barber : s for barber = %d\n",semctl(barber,0, GETVAL,0));


				sleep(3);
				// waiting--;
				sop.sem_num = 0;
				sop.sem_op = -1;
				sop.sem_flg = 0;
				semop(count, &sop, 1);
				exit(0);

			}
		}
	}
	if(mpid == getpid())
	{
		printf("Barber open the shop\n");
		while(semctl(count,0, GETVAL,0) > 0)
		{
			// down barber
			sop.sem_num = 0;
			sop.sem_op = -1;
			sop.sem_flg = 0;
			semop(barber,&sop,1);
			printf("down barber : s for barber = %d\n",semctl(barber,0, GETVAL,0));

			// up customer
			sop.sem_num=0;
			sop.sem_op=1;
			sop.sem_flg=0;
			semop(customer,&sop,1);
			printf("up customer : s for customer = %d\n",semctl(customer,0, GETVAL,0));
			sleep(3);

			//up mutex
			sop.sem_num=0;
			sop.sem_op=1;
			sop.sem_flg=0;
			semop(mutex,&sop,1);
			printf("up mutex : s for mutex = %d\n",semctl(mutex,0, GETVAL,0));
		}
		printf("Barber close the shop\n");
		for(i = 0;i<custom;i++)
		{
			wait(&status);
		}
	}
	semctl(mutex, 0, IPC_RMID, 0);
	semctl(barber, 0, IPC_RMID, 0);
	semctl(customer, 0, IPC_RMID, 0);
	semctl(count, 0, IPC_RMID, 0);
	return 0;
}