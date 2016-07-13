#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/select.h>
#include<string.h>
#include<errno.h>

#define MAXLINE 1024
int main(int argc,char **argv){
	int sockfd,n;
	char recvline[MAXLINE+1];
	struct sockaddr_in servaddr;
	char buff[MAXLINE+1];
	fd_set readset;
	int writefd;
	int readend=0;

	if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1){
		printf("socket error!\n");
		return -1;
	}
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(9999);
	if(inet_pton(AF_INET,"172.16.0.216",&servaddr.sin_addr)<=0){
		printf("errno=%d\n",errno);
		printf("inet_pton error for %s\n",argv[1]);
		exit(1);
	}
	if(connect(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr))<0){
		printf("errno=%d\n",errno);
		printf("connect error!\n");
		return -1;
	}

	writefd=open("write",O_CREAT | O_WRONLY );

	FD_ZERO(&readset);

	while(1){

		if(readend==0){
			FD_SET(0,&readset);
		}else{
			FD_CLR(0,&readset);
		}
		FD_SET(sockfd,&readset);

		select(sockfd+1,&readset,NULL,NULL,NULL);

		if(FD_ISSET(0,&readset)){
			int nread=read(0,buff,MAXLINE);
			if(nread==0){
				printf("we read file at the end!\n");
				readend=1;
				printf("we close sockfd!\n");
				shutdown(sockfd,SHUT_WR);
			}else if(nread>0){
				int nwrite=write(sockfd,buff,nread);
			}
		}

		if(FD_ISSET(sockfd,&readset)){
			printf("we get strecho bytes!\n");
			int nread=read(sockfd,buff,MAXLINE);
			if(nread==0){
				printf("read all strecho bytes!\n");
				close(sockfd);
				return 0;
			}else if(nread>0){
				write(writefd,buff,nread);
			}else{
				perror("read error");
				exit(-1);
			}
		}

	}
	return 0;
}
