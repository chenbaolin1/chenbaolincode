/**********************************
 *文件说明：服务器端主函数
 *
 * *******************************/

int main(int argc, const char *argv[])
{
	int ret, listenfd, connfd, state;
	pthread_t thread;
	char buf[20] = "\0";
	struct sockaddr_in server_addr, client_addr;
	socklen_t addrlen = sizeof(client_addr);

	//设置user root hist_date 的colu 根据数据库的基本格式，最后一个设置为NULL
	const char *col_User[]={USER_COL_NO,USER_COL_ID,USER_COL_PSWD,USER_COL_STATE,USER_COL_NAME,USER_COL_SALA,USER_COL_POSI,USER_COL_TELE,USER_COL_MAIL,USER_COL_ADDR,NULL};

	const char *col_Root[]={ROOT_COL_NO,ROOT_COL_ID,ROOT_COL_PSWD,ROOT_COL_STATE,NULL};

	const char *col_Hist_Date[]={HIST_DATA_COL_NO,HIST_DATA_COL_ID,HIST_DATA_COL_IP,HIST_DATA_COL_TIME,HIST_DATA_COL_DATE,NULL};

	//申请传输结构提
	PRO*command;
	command = (PRO*)malloc(sizeof(PRO));
	if(NULL == command)
	{
	
		perror("malloc failed,error");
		exit(EXIT_FAILURE);
	
	}
	//设置USER,HIST_DATE结构体:
	User = create_struct(PATH_USER,USER_TAB,col_user);
	Root = create_struct(PATH_ROOT,ROOT_TAB,col_root);
	Hist_date =create_struct(PATH_HIST_DATE, NULL,col_hist_date);

	//检查服务器的参数是否正确,
	if(argc != 3)
	{
	
		printf("%s\n",ERROR_IP_PORT);
		exit(1);
	}

	//创建用户数据库
	ret = create_db(User);
	failed(ret, "create_User");
	//创建管理员数据库
	ret = create_db(Root);
	failed(ret, "create_Root");
	//意外退出情况下对用户状态进行初始化
	user_state_init(User,STATE_OFF);
	//创建一个线程供管理员使用
	pthread_create(&thread, NULL, manager_fun,NULL);
	//创建socket()套接子接口
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	failed(listenfd, "socket fail error");
	//设置ip和端口（port)
	memset(&server_addr , '\0', sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	server_addr.sin_addr.s_addr=inet_addr(argv[1]);
	//绑定IP和PORT
	ret = bind(listenfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
	failed(ret, "bind fail error");
	//监听link
	ret = listen(listenfd, 5);
	failed(ret, "listen fail error");
	//设置接收连接请求
	while(1)
	{
	
		//接受客户端请求，返回一个套接字描述符：注意：可读写
		connfd = accept(listenfd, (struct sockaddr *)&client_addr, &addrlen);
		failed(connfd, "connect fail error");
		//接收数据
		memset(command, '\0', sizeof(P));
		ret = recv(connfd, p,sizeof(P),0);
		failed(ret, "recv fail error");

		//客户端请求登录
		if(command->protocol = 0x00000002)
		{
			memset(buf, '\0',sizeof(buf));
			check_user_fun(p->data.Username, buf, &state, User);
			//用户名密码验证
			ret = strcmp(p->data.userpsw, buf);
			//如果成功，则打开创建数据库
			if((o == ret) && (0 == state))
			{
			
				//用户状态修改
				user_state_fun(p->data.username, user, 1);
				HIST_DATE->table = p ->data.username;
				create_db(HIST_DATE);
				add_hist(inet_ntoa(client_addr.sin_addr), HIST_DATE);
				//客户端登录成功返回
				command->protocol= 0x10000012;
				ret = send(connfd, command, sizeof(PRO), 0);
				failed(ret, "send fail error");

				ARG *arg = malloc(sizeof(ARG));
				memset(arg, '\0', sizeof(ARG));
				arg ->sockfd = connfd;
				strcpy(arg->name, command->data.username;);
				//创建线程处理用户请求
				pthread_create(&thread, NULL, login_user, (void *) arg);
				continue;
			
			}
			else if((0 == ret) && (0 != state))
			{
				command->protocol = //重复用户登录状态
			}
		}
	
	}


	
	return 0;
}
