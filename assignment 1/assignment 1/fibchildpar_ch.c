#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
int fibo(int n)
{
	if(n==1)
   {
		return 1;
	}
	else if(n==2)
   {
		return 1;
	}
	else 
   {
		return fibo(n-1) + fibo(n-2);
	}
}

int main ()
{
   int i, n, status;
   int  wtime;
   pid_t cpid, mypid, parpid;
   mypid = getpid();
  
   int a=1,b=1,c;

   printf("How many fibonacci numbers : "); 
   scanf("%d", &n);
   int *arr;
   arr  = (int *)malloc(n*sizeof(int));

   for(i=0;i<n; ++i)
   {
   	cpid = fork();
   	if(cpid == 0)
   	{
         sleep(i);
         c = fibo(i+1);
         printf("%d ",c);
         exit(c);
   	}
   }
   for (i=0; i<n; ++i)
   {
      int which_child = wait(&status);
      arr[which_child-mypid-1] = WEXITSTATUS(status);
   }
   return 0;

}