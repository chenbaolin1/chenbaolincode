/********************************************
 *文件说明：处理用户请求线程
 *
 * *****************************************/
#include "../include/server_include.h"

extern DataBase *User, *Hist_date;

void *login_user(void *p)
{
	ARG arg = *(ARG *)p;
	free(P);
	Hist_date->table = arg.name;

	int ret;
	PRO *command;
	command = (PRO* )malloc(sizeof(PRO));
	if(NULL == command)
	{
		perror("malloc fail error");
		exit(EXIT_FAILURE);
	
	}
	while(1)
	{
		bzero(command, sizeof(PRO));
		ret = recv(arg.sockfd,command,sizeof(PRO), 0);
		//如果数据接收错误
		if(0>ret) 
		{
			break;
		}
		//如果用户掉线
		else if(0 == ret)
		{
			break;
		}
		switch(command -> protocol)
		{
			//查询个人信息
			case 1:
serarch_info:
		
		}
	
	}
}

