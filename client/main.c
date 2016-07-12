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
		//FD_SET(sockfd,&readset);

		select(sockfd+1,&readset,NULL,NULL,NULL);

		if(FD_ISSET(8,&readset)){
			printf("8 is readable!\n");
		}
		if(FD_ISSET(0,&readset)){
			int nread=read(0,buff,1);
			if(nread==0){
				printf("we read file at the end!\n");
				readend=1;
				printf("we close sockfd!\n");
				close(sockfd);//我们在服务器中每次接收到数据之后都要等1秒钟才进行回射，用以模拟网络速度server和client之间的传输时延。当我们读到文件结尾之时，关闭了sockfd，服务器在回射最后一个字节时，由于client端已经关闭了套接字，因此无法正确接收最后的数据
			}else if(nread>0){
				int nwrite=write(sockfd,buff,nread);
				printf("buff[0] is %c\n",buff[0]);
			}
		}

		if(FD_ISSET(sockfd,&readset)){
			printf("we get strecho bytes!\n");
			int nread=read(sockfd,buff,MAXLINE);
			if(nread==0){
				printf("read all strecho bytes!\n");
				close(sockfd);
			}else if(nread>0){
				write(writefd,buff,nread);
				FD_SET(sockfd,&readset);
			}else{
				perror("read error");
			}
		}

		sleep(1);//如果client不是每发一个字节就sleep 1秒，则最后的文件会丢失最后1秒内发送的所有数据；如果每次发送一个字节就sleep 1秒，则会只丢失一个字节。此外，是否丢失这一个字节的数据也和我们selet之后判断的顺序有关系：比如我们的文件只有一个字节a，那么client发送a之后sleep 1秒，而server端在接收到a之后也sleep 1秒，然后再进行回射。因为我们是在同一台机器上测试，则在我们进行select的判断时，0和sockfd几乎是同时满足的，此时如果我们先判断sockfd，则我们可以读取这个字节a，那么最后的文件就没有丢失这个字节；而我们实际上是先判断0，即先判断文件是不是到结尾，此时由于文件已经到结尾，我们将关闭sockfd，而之后由于我们已经关闭了sockfd，导致我们下面对sockfd的read失效（因为此时sockfd已经是错误的文件描述符了）。且由于select对于无效的文件描述符的读监控，一直都会返回可读，即虽然sockfd已经失效了，但是此时我们还在对他进行监控，这就导致select一直返回sockfd可读，从而不停的对sockfd进行read操作，陷入了死循环之中。对此的解决方法就是对49行的FD_SET进行注释，将它的位置改为放在77行。这样死循环的BUG解决，但是还留有数据回射不完全的BUG，下一个版本进行解决。
	}
	return 0;
}
