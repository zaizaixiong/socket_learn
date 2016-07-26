#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<poll.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<errno.h>
#include<signal.h>

#define MAXLINE 1024
#define MAX_OPEN 1024
int main(){
	int listenfd,connfd;
	struct sockaddr_in servaddr;
	char buff[MAXLINE+1];
	int i=0,maxfd;
	int flag[100]={0};//用于标识对应的文件描述符是否需要监控
	struct pollfd client[MAX_OPEN];

	if((listenfd=socket(AF_INET,SOCK_STREAM,0))==-1){
		printf("socket error!\n");
		return -1;
	}
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(9999);
	if(inet_pton(AF_INET,"172.16.0.216",&servaddr.sin_addr)<=0){
		printf("inet_pton error\n");
                exit(1);
	}

	int so_reuseaddr = 1;
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr,sizeof(so_reuseaddr));
	if(bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr))<0){
		printf("bind error!\n");
		return -1;
	}

	listen(listenfd,1);

	for(i=0;i<MAX_OPEN;i++){
		client[i].fd=-1;
	}

	client[0].fd=listenfd;
	client[0].events=POLLRDNORM;

	while(1){

		poll(client,MAX_OPEN,-1);

		if(client[0].revents & POLLRDNORM){
			connfd=accept(listenfd,(struct sockaddr*)NULL,NULL);
			if(connfd>0){
				printf("there is new connection %d\n",connfd);
				client[connfd].fd=connfd;
				client[connfd].events=POLLRDNORM;
			}else if(connfd<0 && errno!=EINTR){
				perror("accept connect error");
				return -1;
			}
		}

		for(i=1;i<MAX_OPEN;i++){
			if(client[i].revents & (POLLRDNORM | POLLERR)){
				printf("i=%d\n",i);
				bzero(buff,MAXLINE);
				int nread=read(i,buff,MAXLINE);
				if(nread>0){
					printf("we read something from %d:%s",i,buff);
					int nwrite=write(i,buff,nread);
					if(nwrite==nread){
						printf("we echo it!\n\n");
					}else{
						printf("we echo error!\n\n");
					}
				}else if(nread==0){
					client[i].fd=-1;
					close(i);
					printf("connection %d closed!\n",i);
				}else if(nread<0){
					if(errno==EINTR){
						continue;
					}else{
						printf("we have error while reading connected %d",i);
						perror("");
						client[i].fd=-1;
						close(i);
					}
				}
			}
		}
	}

	return 0;
}











