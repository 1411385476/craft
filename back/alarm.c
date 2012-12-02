#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>

int limit = 10;
void func();

struct timeval bktime;

void timeout_info(int signo)
{
    if(limit == 0)
    {
        printf("Sorry, time limit reached.\n");
        return;
    }
    printf("only %d senconds left.\n", limit--);
}
 
/* init sigaction */
void init_sigaction(void)
{
    struct sigaction act;
 
    act.sa_handler = timeout_info;
    act.sa_flags   = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGPROF, &act, NULL);
}
 
/* init */
void init_time(void)
{
    struct itimerval val;
 
    val.it_value.tv_sec = 1;
    val.it_value.tv_usec = 0;
    val.it_interval = val.it_value;
    setitimer(ITIMER_PROF, &val, NULL);
}	
int main()
{
	init_sigaction();
	init_time();
	func();
	while(1);
	return 0;
}

void func()
{
	int i;
	gettimeofday(&bktime,NULL);
	srand(time(0));
	i = rand()%5;
	if(i==0) i=1;
	/*unix1;
	unix2;
	unix3;
	i = circle-bktime.tv_sec;
	*/
	printf("%unext circle will start\n",bktime.tv_sec);
	//printf("**********************\n");
	signal(SIGALRM,func);
	alarm(i);
}
