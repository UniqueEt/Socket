#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>


#ifndef CONNECT_SIZE
#define CONNECT_SIZE 256
#endif

#define PORT 7777
#define MAX_LINE 2048

#define MAX_EVENT 20

int max(int a , int b)
{
	return a > b ? a : b;
}

void setNonblocking(int sockfd)
{
	int opts;
	opts = fcntl(sockfd, F_GETFL);
	if (opts<0)
	{
		perror("fcntl(sock,GETFL)");
		return;
	}//if

	opts = opts | O_NONBLOCK;
	if (fcntl(sockfd, F_SETFL, opts)<0)
	{
		perror("fcntl(sock,SETFL,opts)");
		return;
	}//if
}

/*readline函数实现*/
ssize_t readline(int fd, char *vptr, size_t maxlen)
{
	ssize_t	n, rc;
	char	c, *ptr;

	ptr = vptr;
	for (n = 1; n < maxlen; n++) 
	{
		if ( (rc = read(fd, &c,1)) == 1) 
		{
			*ptr++ = c;
			if (c == '\n')
				break;	/* newline is stored, like fgets() */
		}
		else if (rc == 0)
		{
			*ptr = 0;
			return(n - 1);	/* EOF, n - 1 bytes were read */
		} 
		else
			return(-1);		/* error, errno set by read() */
	}

	*ptr = 0;	/* null terminate like fgets() */
	return(n);
}

/*普通客户端消息处理函数*/
void str_cli(int sockfd)
{
	ssize_t n;
	/*发送和接收缓冲区*/
	char sendline[MAX_LINE], recvline[MAX_LINE];
	bzero(sendline, MAX_LINE);

	while (fgets(sendline, MAX_LINE, stdin) != NULL)
	{
		if ((n = write(sockfd, sendline, strlen(sendline))) < 0)
		{
			perror("write error\n");
			exit(-1);
		}
		else
		{
			printf("write %d bytes to server!\n", n);
		}

		bzero(recvline, MAX_LINE);
		if ((n = readline(sockfd, recvline, MAX_LINE)) < 0)
		{
			perror("readline error!\n");
			exit(-1);
		}//if
		else if (n == 0)
		{
			perror("server terminated prematurely\n");
			return;
		}
		else
		{
			printf("read %d bytes from server, message:\n", n);
			if (fputs(recvline, stdout) == EOF || fputc('\n', stdout) == EOF)
			{
				perror("fputs error");
				exit(1);
			}//if
			;
		}
	}//while
}

//void str2_cli(int sockfd)
//{
//	int i, nfds, nread;
//	char sendline[MAX_LINE], recvline[MAX_LINE];
//	struct epoll_event ev, ev_in, event[MAX_EVENT];
//
//	int infd = fileno(stdin);
//
//	int epfd = epoll_create(CONNECT_SIZE);
//
//	setNonblocking(sockfd);
//	/*设置监听描述符*/
//	ev.data.fd = sockfd;
//	/*设置处理事件类型*/
//	ev.events = EPOLLIN | EPOLLET;
//
//	setNonblocking(infd);
//	ev_in.data.fd = infd;
//	ev_in.events = EPOLLIN | EPOLLET;
//	/*注册监听标准输入流stdin事件*/
//	epoll_ctl(epfd, EPOLL_CTL_ADD, infd, &ev_in);
//	/*注册监听连接套接字事件*/
//	epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);
//
//	while (1)
//	{
//		nfds = epoll_wait(epfd, event, CONNECT_SIZE, -1);
//
//		printf("events triggered,nfds = %d\n", nfds);
//
//		for (i = 0; i < nfds; ++i)
//		{
//			if (event[i].data.fd == infd)
//			{
//				if ((nread = read(infd, sendline, MAX_LINE)) == 0)
//				{
//					printf("read nothing\n");
//					close(infd);
//					return;
//				}
//				printf("send message to server\n");
//
//				//即使服务端强制中断（ctrl+z），write还是可以正常向sockfd写入
//				write(sockfd, sendline, nread);
//			}
//			else if (event[i].data.fd == sockfd)
//			{
//				if ((nread = readline(sockfd, recvline, MAX_LINE)) < 0)
//				{
//					perror("read error!\n");
//					return;
//				}
//				//貌似服务端关闭了套接字会向客户端发送EOF,但是如果是服务器强制中断（ctrl+z）,sockfd状态不会改变，走不到这里
//				else if (nread == 0)
//				{
//					printf("server closed prematurely\n");
//					exit(1);
//				}
//				else
//				{
//					printf("receive message from server\n");
//					if (fputs(recvline, stdout) == EOF)
//					{
//						perror("fputs error\n");
//						exit(1);
//					}
//				}
//			}
//		}
//	}
//}

int main(int argc , char **argv)
{
	/*声明套接字和链接服务器地址*/
    int sockfd;
    struct sockaddr_in servaddr;

    /*判断是否为合法输入*/
    if(argc != 2)
    {
        perror("usage:tcpcli <IPaddress>");
        exit(1);
    }//if

    /*(1) 创建套接字*/
    if((sockfd = socket(AF_INET , SOCK_STREAM , 0)) == -1)
    {
        perror("socket error");
        exit(1);
    }//if

    /*(2) 设置链接服务器地址结构*/
    bzero(&servaddr , sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    if(inet_pton(AF_INET , argv[1] , &servaddr.sin_addr) < 0)
    {
        printf("inet_pton error for %s\n",argv[1]);
        exit(1);
    }//if

    /*(3) 发送链接服务器请求*/
    if(connect(sockfd , (struct sockaddr *)&servaddr , sizeof(servaddr)) < 0)
    {
        perror("connect error");
        exit(1);
    }//if

	//调用消息处理函数
	str_cli(sockfd);	
	//调用epoll技术处理消息函数
	//str2_cli(sockfd);

	exit(0);
}
