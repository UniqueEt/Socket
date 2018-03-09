/*******************************************************************************
* 客户端用户基本操作处理实现文件
* 2015-12-10 yrr实现
*
********************************************************************************/

#include "config.h"

/*********************************************
函数名：registerUser
功能：用户注册函数实现
参数：套接字描述符
返回值：成功登陆返回SUCCESS 否则返回异常类型
**********************************************/
int registerUser(int sockfd)
{
	int ret;
	/*声明用户需要的注册信息*/
	User user;
	char buf[MAX_LINE];
	Message message;

	memset(&user, 0, sizeof(user));

	/*获取用户输入*/
	printf("请输入注册用户名：\n");
	scanf("%s" , user.userName);
	printf("user.UserName = %s\n" , user.userName);
	
	printf("请输入注册用户密码：\n");
	scanf("%s" , user.password);
	printf("user.password = %s\n" , user.password);

	//当前用户允许发言
	user.speak = YES;

	memset(buf , 0 , MAX_LINE);
	memcpy(buf , &user , sizeof(user));

	/*发送注册请求消息*/
	if ((ret = send(sockfd, buf, sizeof(buf), 0)) <= 0)
	{
		perror("registerUser, send() failed, server error\n");
		return -1;
	}
	printf("registerUser, send() succeed\n");
	memset(buf , 0 , MAX_LINE);

	/*接收注册响应消息*/
	if ((ret = recv(sockfd, buf, sizeof(buf), 0)) <= 0)
	{
		printf("registerUser, recv() failed, server error: %s\n", errno);
		return -1;
	}
	printf("registerUser, recv() succeed\n");

	memset(&message , 0 , sizeof(message));
	memcpy(&message , buf , sizeof(message));
	
	printf("registerUser,message.content = %s\n",message.content);	
	return message.msgRet;
}
