#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "mmysql.h"

#include "func_handler.h"
#include "server_tcp.h"
#include "promtInfo.h"

int itoa(int num,char* out)
{
    if(out == NULL)
        return -1;
    sprintf(out,"%d",num);
    return 0 ;
}

//服务启关闭后数据处理(清理数据库)
void afterServerClose(int sig)
{
    char sql[1024];
    memset(sql,0x00,sizeof(sql));
    sprintf(sql,"delete from chatUser");

    
	int ret = -1;
    MYSQL* mysql = NULL;
    ret = openMysql(&mysql,"10.130.136.245","root","1","chat",0,NULL,0);
    if(ret != 0)
    {
        printf("MYSQL ERR:%s\n",sql_errInfo);
    }

    ret = sql_query(mysql,sql);
    if(ret != 0)
    {
        printf("MYSQL ERR:%s\n",sql_errInfo);
    }

	closeMysql(mysql);
    exit(0);
}

//获取前缀
int get_prefixInfo(UserInfo *list,int size,const char* IP,const char* fd,char* out_prefix)
{
    if(list==NULL || IP==NULL || fd==NULL || out_prefix==NULL)
        return -1;
    int index ;
    char ret[1024]={0};
    for(index=0; index<size; index++)
    {
        if(strcmp(list[index].IP,"")==0 || strcmp(list[index].fd,"")==0 || strcmp(list[index].IP,"NULL")==0 || strcmp(list[index].fd,"NULL")==0 )
            continue;
        //printf("IP[%s]fd[%s]\n",IP,fd);
        //printf("IP[%s]fd[%s]\n",list[index].IP,list[index].fd);
        if(strcmp(list[index].IP,IP) != 0 || strcmp(list[index].fd,fd) != 0)
            continue;
        sprintf(ret,"[%s|%s]:",list[index].IP,list[index].fd);
        strcpy(out_prefix,ret);
        return 0;
    }
    return -1;
}


//提示信息借口---登陆、下线等
int infoShow(UserInfo* list,int list_size,const char* info)
{
    if(list==NULL || list_size<=0 || info==NULL)
        return -1;

    int i = 0;
    for(i=0; i<1024; i++)
    {
        send(atoi(list[i].fd),info,strlen(info),0);
    }

    return 0;
}

//回调函数入口
void* callBack(void* arg)
{
    PTHREAD_DATA* data = (PTHREAD_DATA*)arg;
    (*data->handler)(&(data->peer));
    if(data->peer.fd = -1)
    {
        data->pth_id = -1;
        data->handler = NULL;
    }
}

//测试接口回调函数
void* deal_func1(void* arg)
{
    //user_01.sql
    PEER_OBJ* var = (PEER_OBJ*)arg;
    struct sockaddr_in addr = var->addr;
    char IP[20] = {0};
    int ret = -1;
    char sql[1024]={0};
    strcpy(IP,inet_ntoa(addr.sin_addr));
    printf("Clined [%s] Connect!\n",IP);
    

    char buf[1024] = {0};
    while(recv(var->fd,buf,sizeof(buf),0) > 0)
    {
        char* buf_tmp = buf;
        printf("get:%s\n",buf_tmp);
        while(*buf_tmp != '\0')
        {
            *buf_tmp = toupper(*buf_tmp);
            buf_tmp++;
        }
        send(var->fd,buf,strlen(buf),0);
        memset(buf,0x00,sizeof(buf));
    }
    char exit_info[1024]="与服务器断开连接!";
    send(var->fd,exit_info,strlen(exit_info),0);
    
    printf("Clined [%s] Close!\n",IP);
    close(var->fd);
    var->fd = -1;
}


//登陆
void* deal_landing(void* arg)
{
    //user.sql
	MYSQL* mysql = NULL;
    char sql[1024]={0};
    char exit_info[1024]="与服务器断开连接!";
	//int ret = openMysql(&mysql,"192.168.52.132","root","1","chat",0,NULL,0);
	int ret = openMysql(&mysql,"10.130.136.245","root","1","chat",0,NULL,0);
	//int ret = openMysql(&mysql,"127.0.0.1","root","1","chat",0,NULL,0);
	if(ret != 0)
    {
        printf("[%s|%d]:[%s]",__FILE__,__LINE__,mysql_error(mysql));
		goto end;
    }
	
	PEER_OBJ* var = (PEER_OBJ*)arg;
    struct sockaddr_in addr = var->addr;
    char IP[20] = {0};
    strcpy(IP,inet_ntoa(addr.sin_addr));
    printf("Clined [%s] Connect!\n",IP);

    memset(sql,0x00,sizeof(sql));
    char str_fd[8]={0};
    itoa(var->fd,str_fd);
    sprintf(sql,"insert into chatUser values('%s','%s','%s','%s')",IP,IP,str_fd,"1");
    ret = sql_query(mysql,sql);
 	if(ret != 0)
 	{
 		printf("MYSQL ERR:%s\n",sql_errInfo);
 	}

    //获取当前所有用户
    UserInfo userInfo[1024];
    memset(userInfo,0x00,sizeof(UserInfo)*1024);
    ret = get_userInfo(mysql,userInfo);
    if(ret != 0)
    {
        printLog(__FILE__,__LINE__,"获取当前登陆用户信息出错");
        goto end;
    }

    //获取用户前缀
    char prefixInfo[1024]={0};
    get_prefixInfo(userInfo,1024,IP,str_fd,prefixInfo);

    //提示新用户登陆
    char info[1024]={0};
    strcpy(info,prefixInfo);
    infoShow(userInfo,1024,strcat(info,LANDING));
    
    //读写
    char buf[2048]={0};
    while(recv(var->fd,buf,sizeof(buf),0) > 0)
    {
        ret = get_userInfo(mysql,userInfo);
        if(ret != 0)
        {
            printLog(__FILE__,__LINE__,"获取当前登陆用户信息出错");
            goto end;
        }

        int i;
        for(i=0; i<1024; i++)
        {
            //前缀
            send(atoi(userInfo[i].fd),prefixInfo,strlen(prefixInfo),0);
            //send(var->fd,buf,strlen(buf),0);
            send(atoi(userInfo[i].fd),buf,strlen(buf),0);
        }
        memset(buf,0x00,sizeof(buf));
    }

end:    
    //提示用户下线
    infoShow(userInfo,1024,strcat(prefixInfo,UNLANDING));

    memset(sql,0x00,sizeof(sql));
    sprintf(sql,"delete from chatUser where IP='%s' and fd='%s'",IP,str_fd);
    ret = sql_query(mysql,sql);
    if(ret != 0)
    {
        printf("MYSQL ERR:%s\n",sql_errInfo);
    }
    //send(var->fd,exit_info,strlen(exit_info),0);
    printf("Clined [%s] Close!\n",IP);
    close(var->fd);
    var->fd = -1;
    
    //关闭数据库
	closeMysql(mysql);
}


