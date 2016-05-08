#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


int main(){
	time_t cur;
	int a[51];
	srand((unsigned) time(&cur));
	int i,c;
	for(i = 0;i<51;i++)
	{
		a[i] = 0;
	}
 	FILE *fp;
 	char name[] = "data_0.txt";

 	for (i=0;i<5;i++){
 		name[5] = (char)(((int)'0')+i+1);
 		fp = fopen(name,"w+");
 		int j = 0;
 		while(j<5)
 		{
 			c = 1+rand()%50;
 			if(a[c] == 0)
 			{
 				fprintf(fp,"%d\t",c);
 				a[c] = 1;
 				j++;
 			}
 		} 		
 	}
	return 0;
}