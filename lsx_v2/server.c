/**********************************
 *  �ļ����ƣ�     �������������server.c 
 *  ������        1.���ݿ��/���������ڴ洢�û���Ϣ���
 *                2.�����׽��֣���IP PORT �����sock�ṹ��
 *                3.�ֶ����root�û��˻�����
 *                5.�������ȴ�����
 *                6.����ͻ��˵����󣬰���root�û�����ɾ�Ĳ飬��ͨ�û����޸�����
 *  
 *      
 *  
 ***********************************/

#include"server.h"
int main(int argc, const char *argv[])
{
	sqlite3 * db;  //���ݿ�
	char *errmsg;   //���ݿ������Ϣ
	int sockfd;   //������׽���
	int acceptfd;  //���յ����׽���
	struct sockaddr_in serveraddr; 
	// USER user;
	pid_t pid;
	//arg ��Ϣ��� IP ��port
	if(argc != 3)
	{
		printf("Usag:%s serverip port\n",argv[0]);
		return -1;
	}

	//�򿪲��������ݿ�
	if(sqlite3_open(DATABASE,&db) != SQLITE_OK)
	{
		printf("%s\n",sqlite3_errmsg(db));
		return -1;
	}

	/*//���������
	  sqlite3_exec(db,"create table data_password (type Integer, , number Integer);",NULL,NULL,&errmsg);*/

	//������Ϣ��
	sqlite3_exec(db,"create table data_info (name text primary key,password char, department char, age char, number Integer,salary Integer,sex char);",NULL,NULL,&errmsg);

	//ע��root   
	regin_root(db);

	//�����׽���
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("make socket error.\n");
		return -1;
	}

	//�Ż�����󶨵�ַ��������
	int b_reuse = 1;
	setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&b_reuse,sizeof(int));

	//���sockreginr_in�ṹ��
	bzero(&serveraddr,sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
	serveraddr.sin_port = htons(atoi(argv[2]));

	//��
	if(bind(sockfd,(struct sockaddr *)&serveraddr,sizeof(serveraddr)) < 0)
	{
		perror("fail to bind");
		return -1;
	}

	//���׽�������Ϊ����ģʽ
	if(listen(sockfd,5) < 0)
	{
		printf("fail to listen\n");
		return -1;
	}

	//����ʬ����
	signal(SIGCHLD,SIG_IGN);

	//���������� 
	while(1)
	{
		//���տͻ�������
		if((acceptfd = accept(sockfd,NULL,NULL)) < 0)
		{
			perror("fail to accept");
			return -1;
		}

		//�����ӽ���
		if((pid = fork()) < 0)
		{
			perror("fail to fork");
			return -1;
		}
		else if(pid == 0)//�ӽ���ִ�в���
		{
			close(sockfd);
			do_client(acceptfd,db);
		}
		else //�����̵ȴ����տͻ�������
		{
			close(acceptfd);
		}       
	}
	return 0;
}

//ע���һ��root�û�
int regin_root(sqlite3 *db)
{
	char * errmsg;
	char sql[N];

	//����root�˺�
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
 *����ͻ��˵���������
 ***********************************/
int do_client(int acceptfd,sqlite3 *db)
{
	 MSG msg;
	while(recv(acceptfd , &msg, sizeof(msg),0) > 0)
	{   
		switch(msg.type)
		{

		case 1://�����û�
			regin(acceptfd,&msg,db);
			break;
		case 2: //��½�û�
			login(acceptfd,&msg,db);
			break; 
		case 3://�����û�
			find(acceptfd,&msg,db);
			break; 
		case 4://�޸��û�����
			modify_password(acceptfd,&msg,db);
			break;

		case 5://ɾ���û�
			delete(acceptfd,&msg,db);
		case 6://�޸��û���Ϣ
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
 *��������½����
 *�������ж��ǹ���Ա������ͨ�û�
 *����ֵ��int
 ***********************************/
int login(int acceptfd, MSG *msg,sqlite3 *db)
{
	int nrow;
	int ncloumn;

	char sql[N];
	char *errmsg;
	char **resultp;
	char admin[] = "0" ;//�����ж��û�����


	//ƥ���û���Ϣ�Ƿ������������ͬ
	sprintf(sql,"select * from data_password where name = '%s' and passwd = '%s';"
			,msg->name,msg->password);
	if(sqlite3_get_table(db, sql, &resultp, &nrow, &ncloumn, &errmsg) != SQLITE_OK)
	{
		printf("%s\n",errmsg);
		return -1;
	}

	//������д��ڸ��û�
	if(nrow == 1)//ӵ�д��û�
	{
		strcpy(msg->data,"login success");//���ͻ��˷��ͻ��Ų�ѯ�ɹ�
		msg->sign = 1; //��������ɹ�

		//�ж��ǹ���Ա������ͨ�û�
		if(strcmp(admin,resultp[ncloumn]) == 0)   //����Ա
		{
			msg->type = 0;
		}
		else   //��ͨ�û�
		{
			msg->type = 1;     
		}
	}

	//������û�������
	if(nrow == 0)
	{
		strcpy(msg->data,"user or password is error,please trt again");
		msg->sign = 0; //�������ʧ��
	}
	//�����û��ȼ���Ϣ
	if(send(acceptfd,msg,sizeof(MSG),0) < 0)
	{
		perror("send failed");
	}   

	return 0;
}

/**********************************
 *����������û�����
 *���������ݿ��ļ���д���û�����
 *
 ***********************************/
int regin(int acceptfd, MSG *msg,sqlite3 *db)
{
	char * errmsg;
	char sql[N];
	int k;

	 //����û���½��Ϣ
	   sprintf(sql,"insert into data_password values('%s','%s',0);"
	   ,msg->name,msg->password);
	   if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK)
	   {
	   strcpy(msg->data,"the user name alread exist,please try again");
	   }
	   else
	   {
	   k = 1;//��ӵ�½��Ϣ�ɹ�
	   strcpy(msg->data,"regin success");
	   }
	   

	//����û���ϸ��Ϣ
	sprintf(sql,"insert into data_info values('%s','%s','%s','%d','%d','%d','%d');"
	,msg->name,msg->password,msg->department,msg->age,msg->number,msg->salary,msg->sex);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK)
	{
		strcpy(msg->data,"the user name alread exist");
	}
	else
	{
		k = k+1;//����û���ϸ��Ϣ�ɹ�
		strcpy(msg->data,"regin success");
	}

	if(k==2)//���߶��ɹ�
	{
		msg->sign = 1; //�ɹ���־
		strcpy(msg->data,"regin success");
	}
	else
	{
		msg->sign = 0;//ʧ�ܱ�־
		strcpy(msg->data,"regin failed!");//ʧ������
	}

	//���Ƿ�ɹ�����Ϣ����ȥ
	if(send(acceptfd,msg,sizeof(MSG),0) < 0)
	{
		perror("send failed");
	}
	return 0;
}

/**********************************
 *�������ı�����ĺ���
 *���������û�����
 ***********************************/
int modify_password(int acceptfd, MSG *msg,sqlite3 *db)
{
	char * errmsg;
	char sql[N];
	int k;
	char **resultp;
	int nrow;
	int ncloumn;

	//������
	if(mutex_front(msg,db) == -1)//���������ֱ�ӷ���
	{
		msg->sign = 0; 
		if(send(acceptfd,msg,sizeof(MSG),0) < 0)
		{
			perror("fail to send");
		}
		return 0;
	}

	//��������
	sprintf(sql,"update data_password set passwd = %s where name = '%s' ",
			msg->password,msg->name);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK)
	{
		printf("change failed!\n");
		strcpy(msg->data,"change faild!");
		msg->sign = 0; //ʧ�ܱ�־       
	}
	else
	{
		strcpy(msg->data,"change success!");
		msg->sign = 1; //�ɹ���־
	}

	//���Ƿ�ɹ�����Ϣ����ȥ
	if(send(acceptfd,msg,sizeof(MSG),0) < 0)
	{
		perror("fail to send");
	}

	//�������
	mutex_after(msg,db);

	return 0;
}

/**********************************
 *��������ѯ��Ϣ����
 *���������ݿ����������в�ѯ���ɹ�1��ʧ��0
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
		msg->sign = 0; //ʧ�ܱ�־
	}
	else
	{
		msg->sign = 1; //�ɹ���־
	}
	strcpy(msg->data,":");
	index = ncloum;
	for(j = 0; j < ncloum; j++)
	{
		strcat(msg->data,resultp[index++]);
		strcat(msg->data," - ");
	}
	//���Ƿ�ɹ�����Ϣ����ȥ
	if(send(acceptfd,msg,sizeof(MSG),0) < 0)
	{
		perror("send failed");
	}
	return 0;
}
/***********************************
 *������ɾ���û�
 *���������ݿ⣬ͨ�����������޸�����
 * *******************************/
int delete(int acceptfd, MSG *msg,sqlite3 *db)
{
	int k = 0;
	int nrow;
	int ncloumn;
	char * errmsg;
	char sql[N];
	char **resultp;

	//������
	if(mutex_front(msg,db) == -1)//���������ֱ�ӷ���
	{
		msg->sign = 0; 
		if(send(acceptfd,msg,sizeof(MSG),0) < 0)
		{
			perror("fail to send");
		}
		return 0;
	}

	/* //ɾ���û���Ӧ�������
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
	   msg->sign = 1; //�ɹ���־       
	   }*/

	//ɾ���û���Ӧ����Ϣ��
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
		msg->sign = 1; //�ɹ���־   
	}
	if(k == 1)
		msg->sign = 0; //ʧ�ܱ�־

	//���Ƿ�ɹ�����Ϣ����ȥ
	if(send(acceptfd,msg,sizeof(MSG),0) < 0)
	{
		perror("fail to send");
	}

	//�������
	mutex_after(msg,db);

	return 0;
}

/**********************************
 *�������޸���Ϣ����
 *���������ݿ⣬�޸�ָ������ݿ���Ϣ�޸�
 ***********************************/
int modify_info(int acceptfd, MSG *msg,sqlite3 *db)
{
	char * errmsg;
	char sql[N];
	int k;
	char **resultp;
	int nrow;
	int ncloumn;

	//������
	if(mutex_front(msg,db) == -1)//���������ֱ�ӷ���
	{
		msg->sign = 0; 
		if(send(acceptfd,msg,sizeof(MSG),0) < 0)
		{
			perror("fail to send");
		}
		return 0;
	}

	//����ǳ����û���ִ�иĹ��ʲ���
	if((msg)->type == 0)
	{
		sprintf(sql,"update data_info set salary = %d where name = '%s' ",msg->salary,msg->name);
		if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK)
		{
			printf("change failed!\n");
			strcpy(msg->data,"change faild!");
			msg->sign = 0; //ʧ�ܱ�־       
		}
		else
		{
			strcpy(msg->data,"change success!");
			msg->sign = 1; //�ɹ���־
		}
	}
	//��ͨ�û��ͳ����û������Ը�������Ϣ
	sprintf(sql,"update data_info set department = '%s' where name = '%s' ",msg->department,msg->name);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK)
	{
		printf("change2 failed!\n");
		strcpy(msg->data,"change faild!");
		msg->sign = 0; //ʧ�ܱ�־       
	}
	else
	{
		strcpy(msg->data,"change success!");
		msg->sign = 1; //�ɹ���־
	}
	//���Ƿ�ɹ�����Ϣ����ȥ
	if(send(acceptfd,msg,sizeof(MSG),0) < 0)
	{
		perror("fail to send");
	}

	//�������
	mutex_after(msg,db);

	return 0;
}

/**********************************
 *������
 ***********************************/
int mutex_front(MSG *msg,sqlite3 *db)
{
	char * errmsg;
	char sql[N];
	char **resultp;
	int nrow;
	int ncloumn;

	//�����⿪ʼ
	sprintf(sql,"select * from data_password where name = '%s' and  number = 1;",msg->name);
	sqlite3_get_table(db, sql, &resultp, &nrow, &ncloumn, &errmsg);

	if(nrow == 1)
	{
		strcpy(msg->data,"huchi");
		msg->sign = 0; //�������ʧ�ܷ���ʧ����Ϣ
		return -1;
	}
	//û�ҵ�˵��û���˲���������ݣ���no��1
	if(nrow == 0)
	{
		sprintf(sql,"update data_password set number = 1 where name = '%s' ",msg->name);
		//����
		sqlite3_exec(db,sql,NULL,NULL,&errmsg);
	}
	return 0;
}

/**********************************
 *�������
 ***********************************/
int mutex_after(MSG *msg,sqlite3 *db)
{
	char * errmsg;
	char sql[N];

	sprintf(sql,"update data_password set number = 0 where name = '%s' ",msg->name);
	sqlite3_exec(db,sql,NULL,NULL,&errmsg);

	return 0;
}
