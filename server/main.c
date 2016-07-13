#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/select.h>
#include<netinet/in.h>
#include<errno.h>
#include<signal.h>

#define MAXLINE 1024
int main(){
	int listenfd,connfd;
	struct sockaddr_in servaddr;
	char buff[MAXLINE+1];
	int i=0,maxfd;
	int flag[100]={0};//用于标识对应的文件描述符是否需要监控
	fd_set readset;

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
	printf("listenfd=%d\n",listenfd);

	int so_reuseaddr = 1;
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr,sizeof(so_reuseaddr));
	if(bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr))<0){
		printf("bind error!\n");
		return -1;
	}

	listen(listenfd,1);

	flag[listenfd]=1;
	maxfd=listenfd;

	FD_ZERO(&readset);

	while(1){
		for(i=0;i<=maxfd;i++){
			if(flag[i]){
				FD_SET(i,&readset);
			}
		}

		select(maxfd+1,&readset,NULL,NULL,NULL);

		for(i=0;i<=maxfd;i++){
			if(FD_ISSET(i,&readset)){
				printf("i=%d\n",i);
				if(i==listenfd){
					connfd=accept(listenfd,(struct sockaddr*)NULL,NULL);
					if(connfd>0){
						printf("there is new connection %d\n",connfd);
						flag[connfd]=1;
						maxfd=maxfd>connfd?maxfd:connfd;
					}else if(connfd<0 && errno!=EINTR){
						perror("accept connect error");
						return -1;
					}
				}else{
					bzero(buff,MAXLINE);
					int nread=read(i,buff,MAXLINE);
					if(nread>0){
						printf("we read something from %d:%s",i,buff);
						int nwrite=write(i,buff,nread);
						if(nwrite==nread){
							printf("we echo it!\n\n");
						}
					}else if(nread==0){
						flag[i]=0;
						/*
						 * 此处的FD_CLR非常重要，如果没有CLR，则下次select的时候i的socket关闭却依然会显示是可读的，会期待i的
						 * 输入，此时会导致程序阻塞。
 						*/
						FD_CLR(i,&readset);
						close(i);
						printf("connection %d closed!\n",i);
					}else if(nread<0){
						if(errno==EINTR){
							continue;
						}else{
							printf("we have error while reading connected %d",i);
							perror("");
							close(i);
						}
					}
				}
			}
		}
	}

	return 0;
}











