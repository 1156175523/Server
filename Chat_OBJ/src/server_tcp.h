#ifndef __SERVER__H__
#define __SERVER__H__

#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <mysql.h>

//服务器端口
#define PORT 10010
//最大链接数
#define BACKLOG 1024

extern MYSQL* mysql;

//对端结构
typedef struct _PEER_OBJ{
    int fd;
    struct sockaddr_in addr;
} PEER_OBJ;

typedef void* (*_handler_t)(void* arg);

//线程数据
typedef struct _PTHREAD_DATA{
    pthread_t pth_id;
    PEER_OBJ peer;
    _handler_t handler;
}PTHREAD_DATA;

int Server_Start(int *sock_fd);
int Server_End(int sock_fd);

void printLog(char* file,int line,char* info);
int Socket();
int Bind(int sock_fd,int port);
int Listen(int sock_fd,int maxnum);
int Accept(int sock_fd);
int Close(int sock_fd);

#endif
