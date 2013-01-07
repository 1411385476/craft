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
#include <fcntl.h>
#include "gnuhash.h"

#define oops(m,x){ perror(m); return; }
#define CACHESIZE 10000
#define MSGLEN 512
#define SOCKNAME "/var/log/log.socket"
//#define LC_DEBUG 0
char db_location[512];
static short int bk_style;//0 nature 1 auto only 2 auto+mand
static short int bk_action;
static short int lc_debug;
pthread_t transfer_tid;
pthread_attr_t transfer_attr;
pthread_t backup_tid;
pthread_attr_t backup_attr;

char log_line[MSGLEN];
char (* log_cache)[MSGLEN];
char log_cache_start[CACHESIZE][MSGLEN];
sem_t *sem;
static int i; //the mark of cache for accept syslog
int sock;
/*the core function of main:
init the arguments and environment
get message from other process (syslog-ng / web)
*/
void * lc_accept();
void * lc_read(int fd, short event, struct event *arg);
int setnonblock(int fd);
void sql_insert(char msg[512]);
void main(int argc,char *argv[])
{
	char sockname[]	= SOCKNAME;
	struct 		sockaddr_un addr;
	socklen_t 	addrlen;
	time_t		now;
	int  msgnum	= 0;
	struct event *ev;
	i = 0;
	/**/
	if(argc>=2){
		lc_debug = 0;
		strcpy(db_location,argv[1]);
	}else{
		lc_debug = 0;
		printf("please point the db for insert log\n");
		return;
	}
	/**/
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
	/* 设置客户端socket为非阻塞模式。 */
    if (setnonblock(sock) < 0)
            oops("failed to set logCraft socket non-blocking",4);
	/*Init libevent*/
	event_init();
	event_set(ev,sock,EV_READ|EV_PERSIST,(void *)lc_accept,ev);
	event_add(ev,NULL);
	event_dispatch();
}

/**
 * 将一个socket设置成非阻塞模式
 */
int
setnonblock(int fd)
{
        int flags;

        flags = fcntl(fd, F_GETFL);
        if (flags < 0)
                return flags;
        flags |= O_NONBLOCK;
        if (fcntl(fd, F_SETFL, flags) < 0)
                return -1;

        return 0;
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
	int client_fd;	
	struct 		sockaddr_un addr;
	socklen_t 	addrlen = sizeof(addr);
	client_fd = accept(sock,(struct sockaddr *) &addr,(socklen_t *)&addrlen);
	if (client_fd < 0) {
        oops("accept",10);
        return;
    }
	if (setnonblock(client_fd) < 0)
            oops("failed to set client socket non-blocking",11);
	struct event *ev = malloc(sizeof(struct event));
    event_set(ev, client_fd, EV_READ|EV_PERSIST, (void *) lc_read, ev);
    event_add(ev, NULL);
}
/*
read syslog from syslog-ng
*/
void * lc_read(int fd, short event, struct event *arg)
{
    int l=0;
	char	msg[MSGLEN];
	l = read(fd,msg,MSGLEN);
	msg[l] = '\0';
	strcpy(*log_cache,msg);
	sql_insert(msg);
	if(lc_debug){
		printf("[sink]: %s",*log_cache);
		fflush(stdout);
	}
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
/*insert sql insto db*/
void sql_insert(msg)
	char msg[512];
{
	int rc,i;
	char msg_temp[600];
	sqlite3 *db;
	rc = sqlite3_open(db_location, &db);
	printf("%s",db_location);
	char * pErrMsg = 0;
	if( rc )
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	}
	else{ 
		//printf("You have opened a sqlite3 database named zieckey.db successfully!nCongratulations! Have fun ! ^-^ n");
		sprintf(msg_temp,"insert into nat(`content`) values('%s');",msg);
		sqlite3_exec( db,msg_temp, 0, 0, &pErrMsg);
	}
	sqlite3_close(db); //关闭
}
