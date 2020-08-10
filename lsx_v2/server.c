/**********************************
 *  文件名称：     服务端主函数：server.c 
 *  描述：        1.数据库打开/创建，用于存储用户信息表格
 *                2.创建套接字，绑定IP PORT ，填充sock结构体
 *                3.手动添加root用户账户密码
 *                5.服务器等待连接
 *                6.处理客户端的请求，包括root用户的增删改查，普通用户的修改密码
 *  
 *      
 *  
 ***********************************/

#include"server.h"
int main(int argc, const char *argv[])
{
	sqlite3 * db;  //数据库
	char *errmsg;   //数据库错误信息
	int sockfd;   //服务端套接字
	int acceptfd;  //接收到的套接字
	struct sockaddr_in serveraddr; 
	// USER user;
	pid_t pid;
	//arg 信息检测 IP 和port
	if(argc != 3)
	{
		printf("Usag:%s serverip port\n",argv[0]);
		return -1;
	}

	//打开并创建数据库
	if(sqlite3_open(DATABASE,&db) != SQLITE_OK)
	{
		printf("%s\n",sqlite3_errmsg(db));
		return -1;
	}

	/*//创建密码表
	  sqlite3_exec(db,"create table data_password (type Integer, , number Integer);",NULL,NULL,&errmsg);*/

	//创建信息表
	sqlite3_exec(db,"create table data_info (name text primary key,password char, department char, age char, number Integer,salary Integer,sex char);",NULL,NULL,&errmsg);

	//注册root   
	regin_root(db);

	//创建套接字
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("make socket error.\n");
		return -1;
	}

	//优化允许绑定地址快速重用
	int b_reuse = 1;
	setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&b_reuse,sizeof(int));

	//填充sockreginr_in结构体
	bzero(&serveraddr,sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
	serveraddr.sin_port = htons(atoi(argv[2]));

	//绑定
	if(bind(sockfd,(struct sockaddr *)&serveraddr,sizeof(serveraddr)) < 0)
	{
		perror("fail to bind");
		return -1;
	}

	//将套接字设置为监听模式
	if(listen(sockfd,5) < 0)
	{
		printf("fail to listen\n");
		return -1;
	}

	//处理僵尸进程
	signal(SIGCHLD,SIG_IGN);

	//并发服务器 
	while(1)
	{
		//接收客户端请求
		if((acceptfd = accept(sockfd,NULL,NULL)) < 0)
		{
			perror("fail to accept");
			return -1;
		}

		//创建子进程
		if((pid = fork()) < 0)
		{
			perror("fail to fork");
			return -1;
		}
		else if(pid == 0)//子进程执行操作
		{
			close(sockfd);
			do_client(acceptfd,db);
		}
		else //父进程等待接收客户端请求
		{
			close(acceptfd);
		}       
	}
	return 0;
}

//注册第一个root用户
int regin_root(sqlite3 *db)
{
	char * errmsg;
	char sql[N];

	//创建root账号
	sprintf(sql,"insert into data_info values('chiguaxiaozu','123456','',0,0,0,'');");
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK)
	{
		printf("create failed,please try agagin or check ip and port\n");
	}
	else
	{
		printf("create success!\n");
	}

	return 0;
}

/**********************************
 *处理客户端的所有请求
 ***********************************/
int do_client(int acceptfd,sqlite3 *db)
{
	 MSG msg;
	while(recv(acceptfd , &msg, sizeof(msg),0) > 0)
	{   
		switch(msg.type)
		{

		case 1://增加用户
			regin(acceptfd,&msg,db);
			break;
		case 2: //登陆用户
			login(acceptfd,&msg,db);
			break; 
		case 3://查找用户
			find(acceptfd,&msg,db);
			break; 
		case 4://修改用户密码
			modify_password(acceptfd,&msg,db);
			break;

		case 5://删除用户
			delete(acceptfd,&msg,db);
		case 6://修改用户信息
			modify_info(acceptfd,&msg,db);
			break;
		default:
			printf("Invalid data msg.\n");
		}
	}

	printf("client exit.\n");
	close(acceptfd);
	exit(0);

	return 0;
}

/**********************************
 *函数：登陆函数
 *描述：判断是管理员还是普通用户
 *返回值：int
 ***********************************/
int login(int acceptfd, MSG *msg,sqlite3 *db)
{
	int nrow;
	int ncloumn;

	char sql[N];
	char *errmsg;
	char **resultp;
	char admin[] = "0" ;//用来判断用户类型


	//匹配用户信息是否与密码表中相同
	sprintf(sql,"select * from data_password where name = '%s' and passwd = '%s';"
			,msg->name,msg->password);
	if(sqlite3_get_table(db, sql, &resultp, &nrow, &ncloumn, &errmsg) != SQLITE_OK)
	{
		printf("%s\n",errmsg);
		return -1;
	}

	//密码表中存在该用户
	if(nrow == 1)//拥有此用户
	{
		strcpy(msg->data,"login success");//给客户端发送回信查询成功
		msg->sign = 1; //代表操作成功

		//判断是管理员还是普通用户
		if(strcmp(admin,resultp[ncloumn]) == 0)   //管理员
		{
			msg->type = 0;
		}
		else   //普通用户
		{
			msg->type = 1;     
		}
	}

	//密码或用户名错误
	if(nrow == 0)
	{
		strcpy(msg->data,"user or password is error,please trt again");
		msg->sign = 0; //代表操作失败
	}
	//返回用户等级信息
	if(send(acceptfd,msg,sizeof(MSG),0) < 0)
	{
		perror("send failed");
	}   

	return 0;
}

/**********************************
 *函数：添加用户函数
 *描述：数据库文件中写入用户数据
 *
 ***********************************/
int regin(int acceptfd, MSG *msg,sqlite3 *db)
{
	char * errmsg;
	char sql[N];
	int k;

	 //添加用户登陆信息
	   sprintf(sql,"insert into data_password values('%s','%s',0);"
	   ,msg->name,msg->password);
	   if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK)
	   {
	   strcpy(msg->data,"the user name alread exist,please try again");
	   }
	   else
	   {
	   k = 1;//添加登陆信息成功
	   strcpy(msg->data,"regin success");
	   }
	   

	//添加用户详细信息
	sprintf(sql,"insert into data_info values('%s','%s','%s','%d','%d','%d','%d');"
	,msg->name,msg->password,msg->department,msg->age,msg->number,msg->salary,msg->sex);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK)
	{
		strcpy(msg->data,"the user name alread exist");
	}
	else
	{
		k = k+1;//添加用户详细信息成功
		strcpy(msg->data,"regin success");
	}

	if(k==2)//两者都成功
	{
		msg->sign = 1; //成功标志
		strcpy(msg->data,"regin success");
	}
	else
	{
		msg->sign = 0;//失败标志
		strcpy(msg->data,"regin failed!");//失败术语
	}

	//将是否成功的消息返回去
	if(send(acceptfd,msg,sizeof(MSG),0) < 0)
	{
		perror("send failed");
	}
	return 0;
}

/**********************************
 *函数：改变密码的函数
 *描述：运用互斥检测
 ***********************************/
int modify_password(int acceptfd, MSG *msg,sqlite3 *db)
{
	char * errmsg;
	char sql[N];
	int k;
	char **resultp;
	int nrow;
	int ncloumn;

	//互斥检测
	if(mutex_front(msg,db) == -1)//如果有锁，直接返回
	{
		msg->sign = 0; 
		if(send(acceptfd,msg,sizeof(MSG),0) < 0)
		{
			perror("fail to send");
		}
		return 0;
	}

	//更新密码
	sprintf(sql,"update data_password set passwd = %s where name = '%s' ",
			msg->password,msg->name);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK)
	{
		printf("change failed!\n");
		strcpy(msg->data,"change faild!");
		msg->sign = 0; //失败标志       
	}
	else
	{
		strcpy(msg->data,"change success!");
		msg->sign = 1; //成功标志
	}

	//将是否成功的消息返回去
	if(send(acceptfd,msg,sizeof(MSG),0) < 0)
	{
		perror("fail to send");
	}

	//互斥解锁
	mutex_after(msg,db);

	return 0;
}

/**********************************
 *函数：查询信息函数
 *描述：数据库以姓名进行查询，成功1，失败0
 ***********************************/
int find(int acceptfd, MSG *msg,sqlite3 *db)
{
	char sql[N] = {};
	char *errmsg;
	char **resultp;
	int nrow;
	int ncloum;
	int i,j;
	int index = 0;
	sprintf(sql,"select *from data_info where name = '%s' ",msg->name);
	if(sqlite3_get_table(db,sql,&resultp,&nrow,&ncloum,&errmsg) != SQLITE_OK)
	{
		printf("%s\n",errmsg);
		msg->sign = 0; //失败标志
	}
	else
	{
		msg->sign = 1; //成功标志
	}
	strcpy(msg->data,":");
	index = ncloum;
	for(j = 0; j < ncloum; j++)
	{
		strcat(msg->data,resultp[index++]);
		strcat(msg->data," - ");
	}
	//将是否成功的消息返回去
	if(send(acceptfd,msg,sizeof(MSG),0) < 0)
	{
		perror("send failed");
	}
	return 0;
}
/***********************************
 *函数：删除用户
 *描述：数据库，通过姓名进行修改密码
 * *******************************/
int delete(int acceptfd, MSG *msg,sqlite3 *db)
{
	int k = 0;
	int nrow;
	int ncloumn;
	char * errmsg;
	char sql[N];
	char **resultp;

	//互斥检测
	if(mutex_front(msg,db) == -1)//如果有锁，直接返回
	{
		msg->sign = 0; 
		if(send(acceptfd,msg,sizeof(MSG),0) < 0)
		{
			perror("fail to send");
		}
		return 0;
	}

	/* //删除用户对应的密码表
	   sprintf(sql,"delete from data_password where name = '%s' ",msg->name);
	   if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK)
	   {
	   printf("change failed!\n");
	   strcpy(msg->data,"change failed!");
	   k = 1;

	   }
	   else
	   {
	   strcpy(msg->data,"delete success!");
	   msg->sign = 1; //成功标志       
	   }*/

	//删除用户对应的信息表
	sprintf(sql,"delete from data_info where name = '%s' ",msg->name);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK)
	{
		printf("delete failed!\n");
		strcpy(msg->data,"delete faild!");
		k = 1;  
	}
	else
	{
		strcpy(msg->data,"delete success!");
		msg->sign = 1; //成功标志   
	}
	if(k == 1)
		msg->sign = 0; //失败标志

	//将是否成功的消息返回去
	if(send(acceptfd,msg,sizeof(MSG),0) < 0)
	{
		perror("fail to send");
	}

	//互斥解锁
	mutex_after(msg,db);

	return 0;
}

/**********************************
 *函数：修改信息函数
 *描述：数据库，修改指令，对数据库信息修改
 ***********************************/
int modify_info(int acceptfd, MSG *msg,sqlite3 *db)
{
	char * errmsg;
	char sql[N];
	int k;
	char **resultp;
	int nrow;
	int ncloumn;

	//互斥检测
	if(mutex_front(msg,db) == -1)//如果有锁，直接返回
	{
		msg->sign = 0; 
		if(send(acceptfd,msg,sizeof(MSG),0) < 0)
		{
			perror("fail to send");
		}
		return 0;
	}

	//如果是超级用户就执行改工资操作
	if((msg)->type == 0)
	{
		sprintf(sql,"update data_info set salary = %d where name = '%s' ",msg->salary,msg->name);
		if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK)
		{
			printf("change failed!\n");
			strcpy(msg->data,"change faild!");
			msg->sign = 0; //失败标志       
		}
		else
		{
			strcpy(msg->data,"change success!");
			msg->sign = 1; //成功标志
		}
	}
	//普通用户和超级用户都可以改以下信息
	sprintf(sql,"update data_info set department = '%s' where name = '%s' ",msg->department,msg->name);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK)
	{
		printf("change2 failed!\n");
		strcpy(msg->data,"change faild!");
		msg->sign = 0; //失败标志       
	}
	else
	{
		strcpy(msg->data,"change success!");
		msg->sign = 1; //成功标志
	}
	//将是否成功的消息返回去
	if(send(acceptfd,msg,sizeof(MSG),0) < 0)
	{
		perror("fail to send");
	}

	//互斥解锁
	mutex_after(msg,db);

	return 0;
}

/**********************************
 *互斥检测
 ***********************************/
int mutex_front(MSG *msg,sqlite3 *db)
{
	char * errmsg;
	char sql[N];
	char **resultp;
	int nrow;
	int ncloumn;

	//互斥检测开始
	sprintf(sql,"select * from data_password where name = '%s' and  number = 1;",msg->name);
	sqlite3_get_table(db, sql, &resultp, &nrow, &ncloumn, &errmsg);

	if(nrow == 1)
	{
		strcpy(msg->data,"huchi");
		msg->sign = 0; //代表操作失败发送失败信息
		return -1;
	}
	//没找到说明没有人操作这个数据，将no置1
	if(nrow == 0)
	{
		sprintf(sql,"update data_password set number = 1 where name = '%s' ",msg->name);
		//上锁
		sqlite3_exec(db,sql,NULL,NULL,&errmsg);
	}
	return 0;
}

/**********************************
 *互斥解锁
 ***********************************/
int mutex_after(MSG *msg,sqlite3 *db)
{
	char * errmsg;
	char sql[N];

	sprintf(sql,"update data_password set number = 0 where name = '%s' ",msg->name);
	sqlite3_exec(db,sql,NULL,NULL,&errmsg);

	return 0;
}
