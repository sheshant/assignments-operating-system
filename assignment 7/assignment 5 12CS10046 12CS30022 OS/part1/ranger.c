#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <semaphore.h>

int jackal[100];
int lion[100];	
int pit[3];
int nj,nl,k,i,j,status;
int count,sema_wait,ranger,mutex,mutex2,lj,jackalwait,lionwait,number;//here
struct sembuf sop;

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
void wakeupall(int pit_no)
{
	int count_wait = semctl(number,0, GETVAL,0);
	semctl(number, 0, SETVAL, 0);
	printf("number of processes waiting = %d\n",count_wait );
	for(i = 0;i<nj;i++) // here
	{
		semctl(jackal[i], 1, SETVAL, pit_no);// first index new pit value
	}
	for(i = 0;i<nl;i++) // here
	{
		semctl(lion[i], 1, SETVAL, pit_no);// first index new pit value
	}
	while(count_wait--)
	{
		up(sema_wait);
	}
}

int main()
{
	key_t key = 0;
	int pit_no,c;
	key = (key_t)206;

	sema_wait = semget(key,1,0666|IPC_CREAT);
	printf("sema_wait %d\n",semctl(sema_wait, 0, GETVAL, 0) );
	key = (key_t)208;  								
	lionwait = semget(key,1,0666|IPC_CREAT);
	key = (key_t)209;  								
	jackalwait = semget(key,1,0666|IPC_CREAT);
	// now up code for both the processes
	up(lionwait);
	up(jackalwait);

	key = (key_t)210;
	mutex = semget(key,1,0666|IPC_CREAT);
	printf("mutex %d\n",semctl(mutex, 0, GETVAL, 0) );
	key = (key_t)220;
	mutex2 = semget(key,1,0666|IPC_CREAT);
	// for thre number of lions and jackals
	key = (key_t)230;
	lj = semget(key,2,0666|IPC_CREAT);
	// semctl(lj, 0, SETVAL, nl);    // lion
	// semctl(lj, 1, SETVAL, 0);	// jackal
	
	key = (key_t)240;
	ranger = semget(key,2,0666|IPC_CREAT);
	// semctl(ranger, 0, SETVAL, 0);
	// semctl(ranger, 1, SETVAL, 0); // index of new pit value
	key = (key_t)260;
	number = semget(key,1,0666|IPC_CREAT);

	
	key = (key_t)201;
	for(i = 0;i<3;i++)
	{
		key = key+1;
		pit[i] = semget(key,5,0666|IPC_CREAT);
	}
	
	
	
	// sleep(1);
	printf("now ranger in wait\n");
	down(ranger);

	key = 0;
	for(i = 0;i<nj;i++)
	{
		key = (key_t)(i+1); 
		jackal[i] = semget(key,6,0666|IPC_CREAT);
	}
	key = 0;
	for(i = 0;i<nl;i++)
	{
		key = (key_t)(i+101); 
		lion[i] = semget(key,6,0666|IPC_CREAT);
	}
	key = (key_t)250;
	count = semget(key,1,0666|IPC_CREAT);
	printf("%d %d\n",count, semctl(count,0, GETVAL,0));
	// now ranger starts
	sleep(1);
	up(lionwait);
	up(jackalwait);

		int given = 0;
		// process will run until all child processes are terminated
		while(semctl(count,0, GETVAL,0) > 0)
			//ranger code
		{
			srand((unsigned int)time(NULL));
			pit_no = rand()%3;c = 0;
			given = 0;
			while(given == 0)
			{
				printf("ranger goes to meat pit %d\n",pit_no );
				if(((semctl(pit[pit_no], 0, GETVAL,0)) <= 40 && // pit is not full
					(semctl(pit[pit_no], 1, GETVAL,0)) == 0 && // there is no jackal
					(semctl(pit[pit_no], 2, GETVAL,0)) == 0)// there is no lion
					|| (semctl(pit[pit_no], 0, GETVAL,0)) == 0)
				{
					// then add meat
					printf("access guaranted for ranger in meat pit %d\n",pit_no );
					down(mutex);
					(semctl(pit[pit_no], 0, SETVAL,(semctl(pit[pit_no], 0, GETVAL,0))+10));
					up(mutex);
					given = 1;
					printf("ranger added food in .................................................%d\n",pit_no );
					down(mutex);
					wakeupall(pit_no);
					up(mutex);
				}
				else // daisy chain
				{
					printf("access denied for ranger in meat pit %d\n",pit_no );
					if(c == 2)
					{
						c = 0;
						printf("ranger wait on meat pit %d\n",pit_no );

						down(ranger);

						pit_no = semctl(ranger,1, GETVAL,0);
						if(pit_no == 10)
						{
							given = 1;
						}
					}
					else
					{
						pit_no = (pit_no+1)%3; c++;
					}
				}
			}
		}
	
	printf("done\n");
	// and then we delete all semaphores
	for(i = 0;i<nl;i++)
	{
		semctl(lion[i], 0, IPC_RMID, 0);
	}
	for(i = 0;i<nj;i++)
	{
		semctl(jackal[i], 0, IPC_RMID, 0);
	}
	for(i = 0;i<3;i++)
	{
		semctl(pit[i], 0, IPC_RMID, 0);
	}
	semctl(count, 0, IPC_RMID, 0);
	semctl(sema_wait, 0, IPC_RMID, 0);
	semctl(lj, 0, IPC_RMID, 0);
	semctl(jackalwait, 0, IPC_RMID, 0);
	semctl(lionwait, 0, IPC_RMID, 0);
	semctl(ranger, 0, IPC_RMID, 0);
	semctl(mutex, 0, IPC_RMID, 0);
	semctl(mutex2, 0, IPC_RMID, 0);
	return 0;
}