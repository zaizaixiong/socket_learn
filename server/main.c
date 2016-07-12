#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<errno.h>
#include<signal.h>

#define MAXLINE 1024
void sig_child(int signo){
	pid_t pid;
	int stat;
	sleep(1);
	while((pid=waitpid(-1,&stat,WNOHANG))>0){
		printf("child %d terminated!\n",pid);
	}
	return;
}

int main(){
	int listenfd,connfd;
	struct sockaddr_in servaddr;
	char buff[MAXLINE+1];
	time_t ticks;
	int pid;

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
	signal(SIGCHLD,sig_child);
	for(;;){
		connfd=accept(listenfd,(struct sockaddr*)NULL,NULL);
		if(connfd<0){
			if(errno==EINTR){
				continue;
			}else{
				perror("accept connect error");
				return -1;
			}
		}
		pid=fork();
		if(pid>0){//father
			printf("child %d connected!\n",pid);
			close(connfd);
		}else if(pid==0){//child
			//printf("it's child process!\n");
			close(listenfd);
			while(1){
				int n=read(connfd,buff,MAXLINE);
				if(0==n){
					close(connfd);
					exit(0);
				}else if(n<0){
					printf("errno=%d\n",errno);
					perror("something error");
					if(errno==EINTR){
						//continue;
					}
					return -1;
				}else{
					sleep(1);//模拟网络速度server和client之间的传输时延
					printf("%d read %d bytes!\n",connfd,n);
					write(connfd,buff,n);
				}
			}
		}else{//error
			printf("something error!\n");
			exit(-1);
		}
	}
	return 0;
}
