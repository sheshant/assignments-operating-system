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
int lion[100];											// here
int pit[3];
int nj,nl,k,i,j,status;
int count,sema_wait,ranger,mutex,mutex2,lj,jackalwait,number;//here
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
	int n;
	int eat_count = 0,pit_no = 0,c,eaten = 0,process = 0;
	printf("n = ");
	scanf("%d",&n);
	printf("(max 100) Total number of jackals = ");  // here
	scanf("%d",&nj);
	pid_t pid,mpid;
	mpid = getpid();
	key_t key = 0;

	for(i = 0;i<nj;i++)
	{
		key = i+1; 
		jackal[i] = semget(key,6,0666|IPC_CREAT);
		semctl(jackal[i], 0, SETVAL, 0);// check setval
		semctl(jackal[i], 1, SETVAL, 0);// first index new pit value
		semctl(jackal[i], 2, SETVAL, 0);// second index position of wait
		semctl(jackal[i], 3, SETVAL, 0);// lion = 0 or jackal = 1
		semctl(jackal[i], 4, SETVAL, 0);// is in wait 		0 if not  // last three doesn;t matter
	}
	key = (key_t)201;
	for(i = 0;i<3;i++)
	{
		key = key+1;
		pit[i] = semget(key,5,0666|IPC_CREAT);
		semctl(pit[i], 0, SETVAL, 50);// pit food value
		semctl(pit[i], 1, SETVAL, 0);// number of lions eating
		semctl(pit[i], 2, SETVAL, 0);// number of jackals eating
		semctl(pit[i], 3, SETVAL, 0);// is ranger waiting
	}
	// for the initial wait of jackal       // here 

	key = (key_t)206;
	sema_wait = semget(key,1,0666|IPC_CREAT);
	semctl(sema_wait, 0, SETVAL, 0);
	key = (key_t)209;  								// here
	jackalwait = semget(key,1,0666|IPC_CREAT);
	semctl(jackalwait, 0, SETVAL, 0);

	key = (key_t)210;
	mutex = semget(key,1,0666|IPC_CREAT);
	semctl(mutex, 0, SETVAL, 1);
	key = (key_t)220;
	mutex2 = semget(key,1,0666|IPC_CREAT);
	semctl(mutex2, 0, SETVAL, 1);
	// for thre number of lions and jackals
	key = (key_t)230;
	lj = semget(key,2,0666|IPC_CREAT);
	// semctl(lj, 0, SETVAL, 0); 	//lion
	semctl(lj, 1, SETVAL, nj);	//jackal
	
	key = (key_t)240;
	ranger = semget(key,2,0666|IPC_CREAT);
	semctl(ranger, 0, SETVAL, 0);
	semctl(ranger, 1, SETVAL, 0); // index of new pit value
	key = (key_t)260;
	number = semget(key,1,0666|IPC_CREAT);
	semctl(number, 0, SETVAL, 0);
	// some operations 							here 
	printf("jackal is waiting for ranger response\n");
	down(jackalwait);

	k = semctl(lj, 0, GETVAL, 0) + semctl(lj, 1, GETVAL, 0); // lion +jackal
	key = (key_t)250;
	count = semget(key,1,0666|IPC_CREAT);
	semctl(count, 0, SETVAL, k);
	

	key = 0;
	for(i = 0;i<nl;i++)
	{
		key = i+101; 
		lion[i] = semget(key,6,0666|IPC_CREAT);
	}
	printf("now the process begins\n");
	// wake up ranger
	up(ranger);

	printf("%d %d\n",count, semctl(count,0, GETVAL,0));
	down(jackalwait);
	for(i = 0;i<nj;i++)  // here
	{
		if((pid = fork()) == 0)
		{
			process = i;
			eat_count = 0;
			while(eat_count<n)
			{
				srand(time(NULL) - i*55);
				pit_no = rand()%3;c = 0;
				eaten = 0;
				while(eaten == 0)
				{
					printf("jackal %d requesting food from meat-pit %d\n",process,pit_no );
					if((semctl(pit[pit_no], 0, GETVAL,0)) == 0)// if pit is empty
					{
						printf("meat pit %d is empty so call to ranger\n",pit_no );
						semctl(ranger,1, SETVAL,pit_no);
						// after wake up
						printf("Jackal %d denied access\n",process);
						up(ranger); // wake up ranger
						if(c == 3) // three round complete
						{
							c = 0;
							printf("Jackal %d in wait queue of meat pit %d\n",process,pit_no);
							down(mutex);
							semctl(number,0,SETVAL,(semctl(number,0,GETVAL,0))+1);
							up(mutex);
							down(sema_wait); // processes waits here 
							pit_no = semctl(jackal[i],1, GETVAL,0);// 1 stores the pitvalue
							printf("Jackal %d is going to meat pit %d\n",process,pit_no);
						}
						else
						{
							pit_no = (pit_no+1)%3; c++;
						}
					}
					else if((semctl(pit[pit_no], 1, GETVAL,0)) > 0 )
					{
						printf("jackal %d denied access\n",process);
						if(c == 2) // three round complete
						{
							c = 0;
							printf("Jackal %d in wait queue of meat pit %d\n",process,pit_no);
							down(mutex);
							semctl(number,0,SETVAL,(semctl(number,0,GETVAL,0))+1);
							up(mutex);
							down(sema_wait); // processes waits here
							pit_no = semctl(jackal[i],1, GETVAL,0);// 1 stores the pitvalue 
							printf("Jackal %d is going to meat pit %d\n",process,pit_no);
						}
						else
						{
							pit_no = (pit_no+1)%3; c++;
						}
					}
					else  // eat it from here
					{
						printf("jackal %d guaranted access\n",process);
						down(mutex);
						(semctl(pit[pit_no], 2, SETVAL,(semctl(pit[pit_no], 2, GETVAL,0))+1));// increase number of jackal
						(semctl(pit[pit_no], 0, SETVAL,(semctl(pit[pit_no], 0, GETVAL,0))-1));// decrement food count
						printf("value of food count is %d at pit no %d\n", (semctl(pit[pit_no], 0, GETVAL,0)),pit_no);
						up(mutex);
						sleep(1);
						// done eating decrement lion or jackal
						eat_count ++;
						eaten = 1;
						printf("eat count for jackal %d = %d\n",process,eat_count);
						down(mutex2);
						(semctl(pit[pit_no], 2, SETVAL,(semctl(pit[pit_no], 2, GETVAL,0))-1));// decrease number of jackal
						up(mutex2);
						printf("Jackal %d left meat pit %d\n",process,pit_no);

						if((semctl(pit[pit_no], 0, GETVAL,0)) <= 40 && // pit is not full
							(semctl(pit[pit_no], 1, GETVAL,0)) == 0 && // there is no jackal
							(semctl(pit[pit_no], 2, GETVAL,0)) == 0) // there is no lion
						{
							// then call ranger
							printf("jackal %d is going to call ranger at meat pit %d\n",process,pit_no );
							semctl(ranger,1, SETVAL,pit_no);
							up(ranger);
						}
							// now call others
						if((semctl(pit[pit_no], 1, GETVAL,0)) == 0 && (semctl(pit[pit_no], 0, GETVAL,0) > 0)) // if there is no lion
						{
							down(mutex);
							printf("jackal %d is going to call all at pit_no %d\n",process,pit_no );
							wakeupall(pit_no);
							up(mutex);
						}
					}
				}
				sleep(2);// sleep after lunch
			}
			down(mutex);
			printf("jackal %d is over \n",process);
			semctl(count,0, SETVAL,semctl(count,0, GETVAL,0)-1);
			if((semctl(count,0, GETVAL,0)) == 0)
			{
				// wakeup ranger
				semctl(ranger,1, SETVAL,10);
				up(ranger);
			}
			up(mutex);
			exit(0);
		}
	}
	for(i = 0;i<nj;i++) // here
	{
		wait(&status);
	}

	return 0;
}