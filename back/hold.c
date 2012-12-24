#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stddef.h>
#include <sys/un.h>
#include <time.h>
#include <signal.h>
#include <semaphore.h>

#define MSGLEN 512
#define CACHESIZE 10000
#define oops(m,x){ perror(m); }
#define SOCKNAME "/var/log/log.socket"

struct arg_set{
	char * log_p;
	sem_t * sem;
};

void * sink(void * p)
{
	int 		sock,client_fd;
	struct 		sockaddr_un addr;
	socklen_t 	addrlen;
	char		msg[MSGLEN];
	int 		i,l;
	char sockname[]	= SOCKNAME;
	time_t		now;
	int  msgnum	= 0;
	char 		* sink_log_cache;
	char 		* sink_log_cache_start;
	sem_t *sem;
	//char 		* sink_log_cache_temp;
	struct arg_set * args = (struct arg_set *)p;

	sink_log_cache = args->log_p;
	sem = args->sem;
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
	client_fd = accept(sock,(struct sockaddr *) &addr,(socklen_t *)&addrlen);
	i = 0;
	sink_log_cache_start = sink_log_cache;
	while(1){
		l = read(client_fd,msg,MSGLEN);
		msg[l] = '\0';
		strcpy(sink_log_cache,msg);
		sem_post(sem);  ///here you need to control rollback of cache with sink
		i++;
		//sink_log_cache_temp = sink_log_cache;
		if(i==CACHESIZE){
			sink_log_cache = sink_log_cache_start;
		}else{
			sink_log_cache += sizeof(msg);
		}
		/*printf("[sink]: %s\n",sink_log_cache_temp);
		fflush(stdout);
		*/
	}
}
