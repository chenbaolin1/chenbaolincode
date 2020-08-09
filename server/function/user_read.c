/***************************************************
 *文件说明：用于读取用户信息
 *
 *
 **************************************************/

#include "../include/server_include.h"

void user_read(const char *file)
{

	FILE *fp;
	size_t ret;
	char buf[1024];
	memset(buf,'/0',sizeof(buf));

	fp = fopen(file, "r");
	if(NULL == fp)
	{
	
		perror("fopen failed");
		exit(EXIT_FAILURE);
	}
	//feof:检测流上的文件结束符
	//ferro:测试错误标识符号
	//两者返回均为0或非0
	while(!feof(fp) && !ferror(fp))
	{
	
		ret = size_t fread(buf, sizeof(buf[0]),sizeof(buf)/sizeof(buf[0],fp));
		 fwrite(buf, sizeof(buf[0], ret, stdout);
		memset(buf, '\0', sizeof(buf));
	
	}
	if(ferror(fp))
	{
		fprintf(stderr , "%s error!\n", file);
		exit(EXIT_FAILURE);


	}
	fclose(fp);


}
