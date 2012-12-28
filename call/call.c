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
	char log[512];
	while(x<20000){
		printf("%d ",x);
		fflush(stdout);
		switch(x%6){
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
			case 4:
                                strcpy(logTag,"SERVICE");
                                break;
			default:
                                strcpy(logTag,"CONFIG");
                                break;
		}
		openlog(logTag,0,LOG_USER);
		sprintf(log,"hellon hello we are very well; id=%d, name=jim, score=100",x);
		syslog(LOG_ERR,log);
		//if(x%1000==0){
		//	sleep(1);
		//}
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
