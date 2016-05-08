#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>


int main ()
{
   int i, n, status;
   pid_t cpid;
   int a=1,b=1; // initial two values of fibonacci series
   printf("How many fibonacci numbers : "); 
   scanf("%d", &n);
   /* here we are creating multiple child processes which is equal to total 
   number of fibonacci numbers to be printed */
   for(i=0;i<n; ++i)
   {
   	  cpid = fork();
   	  if(cpid ==0)
   	  {
   	  	/* in child process we compute the new value and then print 
   	  	it and then return the new computed value after printing it */
   	  	int c  = a+b;
        printf("%d\n",c);
        sleep(1);
   	  	exit(c);
   	  }
   	  else
   	  {
   	  	/* for termination of each child process there is a wait 
   	  	and we can get the value returned by the child process by the macro WEXITSTATUS()*/
   	  	wait(&status);
   	  	a = b;
   	  	b = WEXITSTATUS(status);
   	  	/* the value of a will be equal to the value of b and the value of b will be equal to the value returned 
   	  	so that the sum of a and b will give the next fibonacci number */
   	  }
   }
   return 0;
}