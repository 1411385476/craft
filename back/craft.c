#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stddef.h>
#include <unistd.h>
int bindSocket(char* servname)
{
    struct sockaddr_un usock;
    int ufd;
    memset(&usock,0,sizeof(struct sockaddr_un));
    usock.sun_family = AF_UNIX;
    strcpy(usock.sun_path,servname);
    if((ufd = socket(AF_UNIX,SOCK_STREAM,0)) < 0)
    {
        perror("socket() error!");
        exit(-1);
    }
    int len = offsetof(struct sockaddr_un,sun_path) + strlen(usock.sun_path);
    if(bind(ufd,(struct sockaddr*)&usock,len) < 0)
    {
        perror("bind() error!");
        exit(-1);
    }

    return ufd;
}
int acceptSocket(int ssock,struct sockaddr_un *un)
{
    int csock,len;

    len = sizeof(un); //这句话必须有，因为len是传入传出参数，在传入时如果没有指定大小，有可能传入上次传出参数值。
    if((csock = accept(ssock,(struct sockaddr*)un,(socklen_t*)&len)) < 0)
    {
        perror("accept() error!");
        exit(-1);
    }
    len -= offsetof(struct sockaddr_un,sun_path);
    un->sun_path[len] = 0;
    printf("client file:%s\n",un->sun_path);
    return csock;

}

int main(void)
{
    char servName[20];
    sprintf(servName,"/var/run/log.socket");
    int sfd = bindSocket(servName);
    if(listen(sfd,5) < 0)
    {
        perror("listen() error!");
        exit(-1);
    }
    struct sockaddr_un un;
    int cfd = acceptSocket(sfd,&un);
    char buf[100];
    int i;
    for(i=0;i<10000;i++){
    	int s = read(cfd,buf,sizeof(buf));
   	write(STDOUT_FILENO,buf,s);
    }
}
