#include<stdio.h>
#include<stdlib.h>
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
	char readbuff[MAXLINE+1];
	fd_set readset;
	int len;
	int readfd;

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

	FD_ZERO(&readset);
	FD_SET(0,&readset);
	FD_SET(sockfd,&readset);


	while(select(sockfd+1,&readset,NULL,NULL,NULL)){
		if(FD_ISSET(0,&readset)){
			if(gets(buff)!=NULL){
				len=strlen(buff);
				int nwrite=write(sockfd,buff,len);
				printf("writed %d bytes!\n",nwrite);
			}else{
				printf("we get EOF,so quit!\n");
				close(sockfd);
				exit(-1);
			}
		}
		if(FD_ISSET(sockfd,&readset)){
			int nread=read(sockfd,readbuff,MAXLINE);
			printf("read %d bytes!\n",nread);
		}
		FD_SET(0,&readset);
		FD_SET(sockfd,&readset);
	}
	return 0;
}
