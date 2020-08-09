/*********************************
 *文件说明：创建数据库函数
 *
 *
 * *******************************/

#include "../include/server_include.h"

int create_db(Database *data)
{

	int ret;
	sqlite3 *db;
	char sql[1024] = "\0";
	char **result = NULL;
	int nrow, ncolumn;
	char *errmsg;

	//创建或打开数据库
	ret = sqlite3_open(data->path, &db);
	if(SQLITE_OK != ret)
	{
		fprintf(stderr, "sqlite3_open: %s\n", sqlite3_errmsg(db));
		exit(EXIT_FAILURE);//表示没有成功执行
	}
	/*判断是否为用户数据库*/
	if(!strcmp(PATH_USR, data->path))
	{
	
		memset(sql, '\0', sizeof(sql));
		//字符串格式化命令，将格式化的数据写入字符串中
		sprintf(sql,"create table if not exists %s (%s integer primary key autoincrement, %s text not NULL unique, %s text not NUll, %s integer,
			%s text not NULL, %s double not NULL, %s text not NULL, % double, %s text, %s text);",
				data->table, data->column[0], data->column[1], data->column[2],data->column[3],
				data->column[4],data->column[5],data->column[6],data->column[7],data->column[8],
				data->column[9]);
		ret = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
		if(SQLITE_OK != ret)
		{
		
			//stderr 为标准错误输出，不带缓冲，可以快速输出
			fprintf(stderr, "select failed: %s\n",errmsg);
			exit(EXIT_FAILURE);
		}
		else if(!strcmp(PATH_ROOT, data->path))
		{
		
			//创建root用户数据库
			memset(sql, '\0', sizeof(sql));
			sprintf(sql,"create table if not exists %s (%s integer primary key autoincrement, %s text not NULL unique, %s text not NULL, 
						%s integer not NULL);",
					data->table, data->column[0],data->column[1],data->column[2],data->column[3]);
			ret = sqlite3_exec(db,sql,NULL,NUll, &errmsg);
			if(SQLITE_OK != ret)
			{
				fprintf(stderr, "select failed: %s\n", errmsg);
				exit(EXIT_FAILURE);
			
			}
			//检查是否具有root用户帐号
			
			memset(sql, '\0', sizeof(sql));
			sprintf(sql,"select * from %s;", data->table);
			ret = sqlite3_get_table(db,sql, &result,&nrow, &ncolumn, &errmsg);
			if(SQLITE_OK != ret)
			{
			
				fprintf(stderr, "select failed: %s\n", errmsg);
				exit(EXIT_FAILURE);
			
			}
			//添加root
			if(nrow == 0)
			{
			
				memset(sql, '\0', sizeof(sql));
				sprintf(sql,"create into %s (%s,%s,%s) values (\"%s\",\"%s\", %d);",
						data->table, data->column[1],data->column[2],data->column[3],ROOT_PSW,STATE_OFF);
				ret = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
				if(SQLITE_OK != ok)
				{
				
					fprintf(stderr, "select failed: %s\n", errmsg);
					exit(EXIT_FAILURE);
				
				}
			}

		
		}
		//判断数据库中存在已经录入的信息
		else if(!strcmp(PATH_HIST, data->path))
		{
		
			memset(sql, '\0',sizeof(sql));
			sprintf(sql,"create table if not exists %s (%s integer primary key autoincrement, %s text, %s text, %s text, %s text);",
					data->table, data->column[0],data->column[1],data->column[2],data->column[3],data->column[4]);
			ret = sqlite3_exec(db,sql,NULL,NULL,&errmsg);
			if(SQLITE_OK != ret)
			{
			
				fprintf(stderr, "select failed : %s\n". errmsg);
				exit(EXIT_FAILURE);
			}
		
		}
		else
		{
		
			printf("%s\n",ERROR_SYSTEM);
			exit(EXIT_SUCCESS);
		}
		//关闭数据库
		sqlite3_close(db);




	
	}



}
