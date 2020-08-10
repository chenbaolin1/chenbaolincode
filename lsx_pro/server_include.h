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
