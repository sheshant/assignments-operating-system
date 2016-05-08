#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
int fibo(int n){
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
  
   int a=1,b=1;

   printf("How many fibonacci numbers : "); 
   scanf("%d", &n);
   int *arr;
   arr  = (int *)malloc(n*sizeof(int));
   /* the dynamic array where we are going to store fibonacci numbers in order*/
   for(i=0;i<n; ++i)
   {
   	  cpid = fork();
   	  if(cpid ==0)
   	  {
   	  	exit(fibo(i+1));
   	  	/* child process returning the fibonacci numbers */
   	  }
   }
   for (i=0; i<n; ++i)
   {
      /* the first child will return the 1st fibonacci number which is going to 
      be stored in the index number 0 and the difference between child pid 
      and parent pid is equal to the index value +1  where it is to be stored*/
      int which_child = wait(&status);
      arr[which_child-mypid-1] = WEXITSTATUS(status);
      
   }
   for(i=0;i<n;i++)
   {
   	printf("%d  \n",arr[i] );
   }
   free(arr);
   return 0;

}