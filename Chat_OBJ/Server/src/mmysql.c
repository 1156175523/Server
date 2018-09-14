#include <stdio.h>
#include <mysql.h>
#include <string.h>
#include "mmysql.h"

char sql_errInfo[2048]={0};

int openMysql(MYSQL **mysql,const char *host, const char *user, const char *passwd, const char *db, unsigned int port, const char *unix_socket, unsigned long client_flag)
{
    if(mysql == NULL)
    {
        return -1;
    }
    int ret = -1;
    MYSQL* sql_ret = NULL;
    *mysql = mysql_init(NULL);
    if(*mysql == NULL)
        goto end;
    sql_ret = mysql_real_connect(*mysql,host,user,passwd,db,port,unix_socket,client_flag);
    if(sql_ret == NULL)
        goto end;

    printf(">>>>>>>OPEN MYSQL OK!\n");
    return 0;
end:
    strcpy(sql_errInfo,mysql_error(*mysql));
    return -1;
}

int closeMysql(MYSQL* mysql)
{
    if(mysql == NULL)
        return 0;
    mysql_close(mysql);
    printf(">>>>>>>>close Mysql OK!\n");
}

int sql_query(MYSQL* mysql,const char* sql)
{
    if(mysql == NULL)
        goto end;

    int ret = -1;
    ret = mysql_query(mysql,sql);
    if(ret != 0)
        goto end;

    if(strstr(sql,"select") == NULL)
        return 0;
    MYSQL_RES *res = NULL;
    res = mysql_store_result(mysql);
    if(res == NULL)
        goto end;

    unsigned int num_fields;
    unsigned int i;
    MYSQL_ROW row;
    num_fields = mysql_num_fields(res);
    while ((row = mysql_fetch_row(res)) != NULL)
    {
        for(i = 0; i < num_fields; i++)
        {
            printf("[%s] ", row[i] ? row[i] : "NULL");
        }
        printf("\n");
    }

    mysql_free_result(res);

    return 0;
end:
    strcpy(sql_errInfo,mysql_error(mysql));
    return -1;
}


int get_userInfo(MYSQL *mysql,UserInfo *userList)
{
    if(NULL == mysql || NULL == userList)
        goto end;

    char sql[1024]="select * from chatUser";

    mysql_query(mysql,sql);

    MYSQL_RES* res = NULL;
    res = mysql_store_result(mysql);
    if(NULL == res)
        goto end;

    int num_fields = mysql_num_fields(res);
    MYSQL_ROW row;
    int index = 0;
    while((row=mysql_fetch_row(res)) != NULL)
    {   
        /*
        int i;
        for(i=0; i<num_fields; i++)
        {   
            //printf("[%s] ",row[i]!=NULL?row[i]:"NULL");
        } 
        printf("\n");
        */

        strcpy(userList[index].IP,row[0]!=NULL?row[0]:"NULL");
        strcpy(userList[index].name,row[1]!=NULL?row[1]:"NULL");
        strcpy(userList[index].fd,row[2]!=NULL?row[2]:"NULL");
        index++;
    } 

    mysql_free_result(res);

    return 0;
end:
    return -1;
}
