/****************************************
 *文件名称：函数声明结构体文件
 *描述：   1. 定义双方通信信息的结构体1成功 0失败
 *         2. 登录信息的结构体
 *         3. 员工信息的结构体
 *         4. 函数声明
 *
 *
 * *************************************/

#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<signal.h> 
#include<string.h>
#include<unistd.h>
#include<sqlite3.h>
#define DATABASE "user.db"
#define N 500 


/*typedef struct  //登陆信息
{
	int  type;//判断是否为管理员
	char name[N];  //姓名
	char passwd[N];  //密码
	int no;//工号
}USER;

typedef struct   //员工信息
{
	char name[N];//姓名
	char addr[N];//地址 
	int  age;//年龄
	int  no;//工号
	double  salary;//工资
	char phone[N];//手机号
	int type;//员工级别

}INFO;

typedef struct{   //传输信息结构体
	int sign; //操作成功还是失败
	int type;//判断操作类型 
	USER user; //员工登陆信息
	INFO info;//员工信息结构体
	char data[N];//操作成功 失败的消息
}MSG;
*/

typedef struct{
	int type; //
	char name[N];//姓名
	int age;//年龄
	char sex;//性别
	char password[N];//密码
	int number;//编号
	int salary;//工资
	char department[N];//部门
	char data[N];//是否成功
	int sign;//操作成功还是失败

	

}MSG;
/*}__attribute__((packed)) MSG;*/




//注册第一个root用户
int register_root(sqlite3 *db);
//处理函数
int do_client(int acceptfd,sqlite3 *db);
//登陆函数
int login(int acceptfd, MSG *msg,sqlite3 *db);
//增加函数
int regin(int acceptfd, MSG *msg,sqlite3 *db);
//修改密码函数
int modify_password(int acceptfd, MSG *msg,sqlite3 *db);
//查找函数
int find(int acceptfd, MSG *msg,sqlite3 *db);
//删除函数
int delete(int acceptfd, MSG *msg,sqlite3 *db);
//修改信息函数
int modify_info(int acceptfd, MSG *msg,sqlite3 *db);
//互斥检测
int mutex_front(MSG *msg,sqlite3 *db);
//解锁
int mutex_after(MSG *msg,sqlite3 *db);
