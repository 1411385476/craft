/*the src is the main part of logCraft
logCraft: a software for process Log from system
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stddef.h>
#include <sys/un.h>
#include <event.h>
#include <sqlite3.h>
#include <time.h>
#include <signal.h>
#include <semaphore.h>
#include "gnuhash.h"

#define oops(m,x){ perror(m); return; }
#define CACHESIZE 10000
#define MSGLEN 512
#define SOCKNAME "/var/log/log.socket"
static short int bk_style;//0 nature 1 auto only 2 auto+mand
static short int bk_action;
pthread_t transfer_tid;
pthread_attr_t transfer_attr;
pthread_t backup_tid;
pthread_attr_t backup_attr;

char log_line[MSGLEN];
char (* log_cache)[MSGLEN];
char log_cache_start[CACHESIZE][MSGLEN];
sem_t *sem;
int i; //the mark of cache for accept syslog
int sock,client_fd;
struct 		sockaddr_un addr;
socklen_t 	addrlen;
/*the core function of main:
init the arguments and environment
get message from other process (syslog-ng / web)
*/
void * lc_accept();
main()
{
	char sockname[]	= SOCKNAME;
	time_t		now;
	int  msgnum	= 0;
	struct event *ev;
	i = 0;
	log_cache		= log_cache_start;
	ev = (struct event *)malloc(sizeof(struct event));
	/**/
	addr.sun_family	= AF_UNIX;
	strcpy(addr.sun_path,sockname);
	
	sock	= socket(AF_UNIX,SOCK_STREAM,0);
	
	addrlen = offsetof(struct sockaddr_un, sun_path)+ strlen(addr.sun_path);
	
	unlink(sockname);
	
	if(bind(sock,(struct sockaddr *) &addr,addrlen) == -1){
		oops("bind",3);
	}
	listen(sock,6);
	/*Init libevent*/
	event_init();
	event_set(ev,sock,EV_READ|EV_PERSIST,(void *)lc_accept,ev);
	event_add(ev,NULL);
	event_dispatch();
}
/*
	init the process
*/
void lc_init(){

}
/*
	callback function for process the messge from web
*/
void lc_ctl(){
	
}
/*this is the function for receive syslog
callback function
*/
void * lc_accept(){
	int l;
	char	msg[MSGLEN];
	client_fd = accept(sock,(struct sockaddr *) &addr,(socklen_t *)&addrlen);
	l = read(client_fd,msg,MSGLEN);
	msg[l] = '\0';
	strcpy(*log_cache,msg);
	printf("[sink]: %s",*log_cache);
	fflush(stdout);
	//logcache[] = msg;
	//sem_post(sem);  ///here you need to control rollback of cache with accept
	if(i==CACHESIZE){
		log_cache = log_cache_start;
	}else{
		log_cache++;
	}
	i++;
}
/*
thread for process log (main function)
this is the function for process syslog
*/
void lc_transfer(){
}
/*
backup thread`s main function
usage:which is uesd to backup those main database
*/
void lc_backup(){
}
