#include <stdio.h> 
#include <stdlib.h>
#include <string.h> 
#include "sys/syslog.h"

int sort_function( const void *a, const void *b); 
int list[5] = { 54, 21, 11, 67, 22 }; 

int main(void) 
{ 
	int x=0;
	//openlog("TRAFNAT",0,LOG_USER);
	char logTag[20];
	while(x<10000){
		printf("%d ",x);
		fflush(stdout);
		switch(x%5){
			case 0: 
				strcpy(logTag,"TRAFNAT");
				break;
			case 1:
				strcpy(logTag,"TRAFSESSION");
                                break;
			case 2:
                                strcpy(logTag,"SECURITY");
                                break;
			case 3:
                                strcpy(logTag,"AUDIT");
                                break;
			default:
                                strcpy(logTag,"CONFIG");
                                break;
		}
		openlog(logTag,0,LOG_USER);
		syslog(LOG_ERR,"hello we are very well; id='111', name='jim', score='100'");
		sleep(1);
		x++;
	}
//qsort((void *)list, 5, sizeof(list[0]), sort_function); 
//for (x = 0; x < 5; x++) 
//printf("%i\n", list[x]); 
return 0; 
} 

int sort_function( const void *a, const void *b) 
{ 
return *(int*)a-*(int*)b; 
}
