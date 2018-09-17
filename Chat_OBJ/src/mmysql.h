#ifndef __MMYSQL_H_
#define __MMYSQL_H_
#include "func_handler.h"
#include <mysql.h>

extern char sql_errInfo[2048];

//打开数据库
int openMysql(MYSQL **mysql,const char *host, const char *user, const char *passwd, const char *db, unsigned int port, const char *unix_socket, unsigned long client_flag);

//关闭数据库
int closeMysql(MYSQL* mysql);

//数据库操作
int sql_query(MYSQL* mysql,const char* sql);

//获取用户信息
int get_userInfo(MYSQL* mysql,UserInfo *userList);

#endif
