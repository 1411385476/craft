/*the src is the main part of logCraft
logCraft: a software for process Log from system
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <stddef.h>
#include <sys/un.h>
#include <event.h>
#include <sqlite3.h>
#include <time.h>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>
#include <syslog.h>
#include "gnuhash.h"

#define oops(m,x){ perror(m); return; }
#define CACHESIZE 10000
#define MSGLEN 512
#define MAXLINE 1024
#define DEFUPRI		(LOG_USER|LOG_NOTICE)
#define DEFSPRI		(LOG_KERN|LOG_CRIT)
#define SOCKNAME "/var/log/log.socket"

static int	Debug;			/* debug flag */
static int debugging_on = 0;
static int lt_debugging_on = 0;

char db_location[512];
static short int bk_style;//0 nature 1 auto only 2 auto+mand
static short int bk_action;
static int cache_rflag = 0;
pthread_t transfer_tid;
pthread_attr_t transfer_attr;
pthread_t backup_tid;
pthread_attr_t backup_attr;

char log_line[MSGLEN];
char (* log_cache)[MSGLEN];
char (* log_cache_out)[MSGLEN];
char log_cache_start[CACHESIZE][MSGLEN];
sem_t *sem;
int sock;
char * parts;//the left part of message
struct lc_statistic{
	int lc_receive;
	int lc_drop;
	int lc_bad; //wrong format
	int lc_db_succ;
	int lc_db_fail;
} lc_stat;


/*function list*/
void * lc_accept();
void * lc_read(int fd, short event, struct event *arg);
int setnonblock(int fd);
void updateLog(char *msg,int len);
void printchopped(char*,int);
void printline(char *msg); //insert log into cache

void lc_transfer();
void assemLog(char (*msg)[MSGLEN],char sline[MSGLEN+100]);
void sql_insert(char (*msg)[MSGLEN]);

static void lc_dprintf(char *, ...);//output main info
static void lt_dprintf(char *fmt, ...); //output transfer info
void logerror(char *type); //output syslog


/*the core function of main:
init the arguments and environment
get message from other process (syslog-ng / web)
*/
void main(int argc,char *argv[])
{
	char sockname[]	= SOCKNAME;
	struct 		sockaddr_un addr;
	socklen_t 	addrlen;
	time_t		now;
	int  msgnum	= 0;
	struct event *ev;
	/**/
	if(argc>=2){
		Debug = 0;
		strcpy(db_location,argv[1]);
	}else{
		Debug = 0;
		printf("please point the db for insert log\n");
		return;
	}
	if(argc>=3){
		Debug = 1; //Debug turning
		debugging_on = 0; //output the main thread debug info
		lt_debugging_on =1; //output the transfer thread debug info
	}
	/**/
	log_cache		= log_cache_start;
	log_cache_out	= log_cache_start;
	lc_stat.lc_receive = 0;	
	lc_stat.lc_drop = 0;
	lc_stat.lc_bad = 0;
	lc_stat.lc_db_succ = 0;
	lc_stat.lc_db_fail = 0;
	ev = (struct event *)malloc(sizeof(struct event));
	/**/
	parts = (char *) 0;
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
	/*start the transfer thread*/
	pthread_attr_init(&transfer_attr);
	pthread_create(&transfer_tid,&transfer_attr,lc_transfer,NULL);
	/*Init libevent*/
	event_init();
	event_set(ev,sock,EV_READ|EV_PERSIST,(void *)lc_accept,ev);
	event_add(ev,NULL);
	event_dispatch();
}

/**
 * set this socket be nonblock
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
            oops("failed to set client socket non-blocking\n",11);
	struct event *ev = malloc(sizeof(struct event));
    event_set(ev, client_fd, EV_READ|EV_PERSIST, (void *) lc_read, ev);
    event_add(ev, NULL);
}
/*
read syslog from syslog-ng
*/
void * lc_read(int fd, short event, struct event *arg)
{
    int len=0;
	char	msg[MAXLINE+1];
	memset(msg, 0, sizeof(msg));
	len = read(fd,msg,MAXLINE-2);
	lc_dprintf("^^^^^^^^^^^^^^^^^^^^^\n%s\n~~~~~~~~~~~~~~~~~~~~~\n",msg);
	updateLog(msg,len);
	printchopped(msg, len);
}

/*chopped the log into line*/
void printchopped(msg, len)
	char *msg;
	int len;
{
	auto int ptlngth;

	auto char *start = msg,*p,*end,tmpline[MAXLINE + 1];

	lc_dprintf("Message length: %d.\n", len);
	tmpline[0] = '\0';
	if ( parts != (char *) 0 )
	{
		lc_dprintf("Including part from messages.\n");
		strcpy(tmpline, parts);
		free(parts);
		parts = (char *) 0;
		lc_dprintf("###########begining#################\n");
		lc_dprintf("%s\n",msg);
		lc_dprintf("***********templine list************\n");
		lc_dprintf("%s\n",tmpline);
		lc_dprintf("&&&&&&&&&&&msg list&&&&&&&&&&&&&&&&&\n");
		if ( (strlen(msg) + strlen(tmpline)) > MAXLINE )
		{
			logerror("Cannot glue message parts together\n");
			printline(tmpline);
			start = msg;
		}
		else
		{
			lc_dprintf("Previous: %s\n", tmpline);
			lc_dprintf("Next: %s\n", msg);
			strcat(tmpline, msg);	/* length checked above */
			printline(tmpline);
			if ( (strlen(msg) + 1) == len )
				return;
			else
				start = strchr(msg, '\0') + 1;
		}
	}

	if ( msg[len-1] != '\0' )
	{
		msg[len] = '\0';
		/*get the left of log,then storing it into parts*/
		for(p= msg+len-1; *p != '\0' && p > msg; )
			--p;
		if(*p == '\0') p++;
		ptlngth = strlen(p);
		if ( (parts = malloc(ptlngth + 1)) == (char *) 0 )
			logerror("Cannot allocate memory for message part.\n");
		else
		{
			strcpy(parts, p);
			lc_dprintf("Saving partial msg: %s\n", parts);
			memset(p, '\0', ptlngth);
		}
	}

	do {
		end = strchr(start + 1, '\0');
		printline(start);
		start = end + 1;
	} while ( *start != '\0' ); //'\0' because memset(p, '\0', ptlngth); and len+2

	return;
}
/*
 * change \n to \0
 * 
 */

void updateLog(msg,len)
	char *msg;
	int len;
{
	register char *p, *q;
	register unsigned char c;
	int mark;

	p = msg;
	/*add by lizxf*/
	while ((c = *p)&&p<msg+len) {
		if (c == '\n' || c == 127){
			q = p+1;
			if(q==msg+len){
				*p = '\0';
			}else if (*q == '<') {
				mark = 0;
				*p = ' ';
				while (isdigit(*++q)&&q<msg+len)
				{
				   		mark = 2;
				}
				if(q==msg+len||(mark == 2&&*q == '>')){
							*p = '\0';
				}
			}else{
				*p = ' ';
			}
		}
		p++;
	}
	return;
}
/*insert the log into share memory*/
void printline(msg)
	char * msg;
{
	lc_stat.lc_receive++;
	if(log_cache_out-log_cache==1){
		lc_stat.lc_drop++;
		return;
	}else{
		memset(*log_cache,0,sizeof(*log_cache));
		strcpy(*log_cache,msg);
		log_cache = log_cache_start + (++log_cache - log_cache_start)%CACHESIZE;
	}
	lc_dprintf("[sink]: %s\n",*log_cache);

	return;
}


/*
thread for process log (main function)
this is the function for process syslog
*/
void lc_transfer(){
	char cline[MSGLEN];
	char sline[MSGLEN+100];
	
	while(1){
		/*get msg from cache*/
		if(log_cache-log_cache_out==0){
			continue;
		}
		memset (cline, 0, sizeof(cline));
		memset (sline, 0, sizeof(sline));
		strcpy(cline,*log_cache_out);
		log_cache_out = log_cache_start + (++log_cache_out - log_cache_start)%CACHESIZE;
		lt_dprintf("%s\n",cline);
		
		/*check and split the message assem it into a sql sentence*/
		assemLog(cline,sline);
		/*insert message into db*/
		sql_insert(sline);
	}
	return;
}

/*
 * Take a raw input line, decode the message, and print the message
 * on the appropriate log files.
 */

void assemLog(msg,smsg)
	char (*msg)[MSGLEN];
	char (*smsg)[MSGLEN+100];
{
	register char *p, *q;
	register unsigned char c;
	int pri;

	p = msg;

	if (*p == '<') {
		pri = 0;
		while (isdigit(*++p))
		{
		   pri = 10 * pri + (*p - '0');
		}
		if (*p == '>')
			++p;
	}

	q = smsg;
	while ((c = *p++) && q < (smsg+sizeof(smsg) - 4)) {
		if (c < 040) {
			*q++ = '^';
			*q++ = c ^ 0100;
		} else
			*q++ = c;
	}
	*q = '\0';

	return;
}

/*
backup thread`s main function
usage:which is uesd to backup those main database
*/
void lc_backup(){


	
}


/*insert sql insto db*/
void sql_insert(msg)
	char (*msg)[MSGLEN];
{
	int rc,i;
	char msg_temp[MSGLEN+100];
	sqlite3 *db;
	rc = sqlite3_open(db_location, &db);
	lc_dprintf("%s",db_location);
	char * pErrMsg = 0;
	if( rc )
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	}
	else{ 
		//lt_dprintf("You have opened a sqlite3 database \n");
		sprintf(msg_temp,"insert into nat(`content`) values('%s');",*msg);
		sqlite3_exec( db,msg_temp, 0, 0, &pErrMsg);
	}
	sqlite3_close(db); //关闭
}
/*log error from logCraft*/
void logerror(type)
	char *type;
{
	char buf[100];

	lc_dprintf("Called logerr, msg: %s\n", type);
	/*produce the its error syslog we need to edit the time and host*/
	if (errno == 0)
		(void) snprintf(buf, sizeof(buf), "LOG_CRAFT: %s", type);
	else
		(void) snprintf(buf, sizeof(buf), "LOG_CRAFT: %s: %s", type, strerror(errno));
	errno = 0;
	//logmsg(LOG_SYSLOG|LOG_ERR, buf, LocalHostName, ADDDATE);
	printline(buf);
	return;
}
/*print the debug infomation*/
static void lc_dprintf(char *fmt, ...)

{
	va_list ap;

	if ( !(Debug && debugging_on) )
		return;
	
	va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
	va_end(ap);

	fflush(stdout);
	return;
}
/*print the transfer log info*/
static void lt_dprintf(char *fmt, ...)

{
	va_list ap;

	if ( !(Debug && lt_debugging_on) )
		return;
	
	va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
	va_end(ap);

	fflush(stdout);
	return;
}

