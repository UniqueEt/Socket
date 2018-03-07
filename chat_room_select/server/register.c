/*******************************************************************************
* 服务器处理用户基本操作处理实现文件
* 2015-12-10 yrr实现
*
********************************************************************************/

#include "config.h"

/*********************************************
函数名：registerUser
功能：用户注册函数实现
参数：msg--用户发送的注册消息 sockfd--套接字描述符
返回值：成功登陆返回SUCCESS 否则返回异常类型
**********************************************/
int registerUser(Message *msg , int sockfd)
{
	int ret;
	/*声明用户需要的注册信息*/
	User user;
	char buf[MAX_LINE];
	
	/*声明数据库变量*/
	sqlite3 *db;
	sqlite3_stmt *stmt;
	const char *tail;

	/*声明sql语句存储变量*/
	char sql[1024];

	/*当前系统时间*/
	time_t timeNow;

	/*存储操作结果消息*/
	Message message;

	memset(&user , 0 , sizeof(user));
	memset(buf, 0, sizeof(buf));

	/*接收用户注册信息*/
	recv(sockfd , buf , sizeof(buf) , 0);

	memcpy(&user , buf , sizeof(user));
	user.userAddr = (*msg).sendAddr;
	user.sockfd = sockfd;
	
	if(strlen(user.userName) > 20)
	{	
		fprintf(stderr, "register failed, userName must less than or equal to 20 bytes\n");
		return INVALID;
	}//if

	/*（1）打开数据库*/
	ret = sqlite3_open(DB_NAME, &db);
	if(ret != SQLITE_OK)
	{
		fprintf(stderr, "sqlite3_open failed, [%s]\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return FAILED;
	}//if
	printf("Opened database successfully.\n");

	/*（2）检查要注册用户名是否已存在？*/
	memset(sql , 0 , sizeof(sql));
	sprintf(sql , "select * from user where userName='%s';",(user.userName));

	/*
	 *Before an SQL statement is executed, it must be first compiled into a byte-code with one of the sqlite3_prepare* functions.
	 *(The sqlite3_prepare() function is deprecated.)
	*/
	ret = sqlite3_prepare_v2(db , sql , strlen(sql) , &stmt , &tail);	
	if(ret != SQLITE_OK)
	{
		fprintf(stderr, "1 sqlite3_prepare_v2 failed, [%s]\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return FAILED;
	}//if

	/*
	 * runs the SQL statement
	 * Our SQL statement returns only one row of data, therefore, we call this function only once
	*/
	ret = sqlite3_step(stmt);
	if (ret == SQLITE_ROW)
	{
		fprintf(stderr, "1 sqlite3_step failed, [%s]\n", sqlite3_column_text(stmt, 0));
		printf("该用户已存在\n");
		sqlite3_close(db);
		return FAILED;
	}
	/*销毁句柄，关闭数据库*/
	sqlite3_finalize(stmt);

	/*执行插入操作*/
	memset(sql , 0 , sizeof(sql));
	time(&timeNow);
	sprintf(sql , "insert into user(userName , password , userAddr , sockfd , speak , registerTime)\
			values('%s','%s','%s',%d, %d , '%s');",user.userName , user.password , 
			inet_ntoa(user.userAddr.sin_addr),user.sockfd , YES, asctime(gmtime(&timeNow)));

	printf("insert sql = %s\n", sql);

	/*第二次调用该函数之前要调用sqlite3_finalize销毁stmt*/
	ret = sqlite3_prepare_v2(db , sql , strlen(sql) , &stmt , &tail);	
	if(ret != SQLITE_OK)
	{
		fprintf(stderr, "2 sqlite3_prepare_v2 failed, [%s]\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return FAILED;
	}//if
	
	ret = sqlite3_step(stmt);
	if (ret == SQLITE_ROW)
	{
		fprintf(stderr, "2 sqlite3_step failed, [%s]\n", sqlite3_column_text(stmt, 0));
		printf("插入用户数据失败\n");
		sqlite3_close(db);
		return FAILED;
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);
		
	return SUCCESS;
}
