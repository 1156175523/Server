//TCP Server

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "server_tcp.h"
#include "func_handler.h"

PTHREAD_DATA pthreadData[BACKLOG];


//初始化连接队列
int init_list()
{
	int i = 0;
	for(; i<BACKLOG; i++)
	{
		pthreadData[i].pth_id = -1;
		pthreadData[i].peer.fd = -1;
		memset(&(pthreadData[i].peer.addr),0x00,sizeof(pthreadData[i].peer.addr));
		pthreadData[i].handler = NULL;
	}
	return 0;
}

//查看链接队列
int check_list(PTHREAD_DATA* list)
{
    if(list == NULL)
        return -1;
    int index = 0;
    while(index < BACKLOG)
    {
        if((list+index)->pth_id == -1)
            break;
        index++;
    }
    if(index > BACKLOG)
        return BACKLOG;
    return index;
}

void printLog(char* file,int line,char* info)
{
    printf("[%s|%d] %s\n",file,line,info);
}

int Socket()
{
    int sock_fd = socket(AF_INET,SOCK_STREAM,0);
    if(sock_fd == -1)
        return -1;
    return sock_fd;
}

int Bind(int sock_fd,int port)
{
    struct sockaddr_in serv_addr;
    bzero(&serv_addr,sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    socklen_t tmp = 1;
    int ret = setsockopt(sock_fd,SOL_SOCKET,SO_REUSEADDR,&tmp,sizeof(socklen_t));
    if(ret == -1) return -1;

    ret = bind(sock_fd,(struct sockaddr*)&serv_addr,sizeof(struct sockaddr));
    if(ret == -1) return -1;

    
    return 0;
}

int Listen(int sock_fd,int maxnum)
{
    int ret = listen(sock_fd,maxnum);
    if(ret == -1)
        return -1;
    return 0;
}

int Accept(int sock_fd)
{
    struct sockaddr_in clie_addr;
    bzero(&clie_addr,sizeof(clie_addr));
    socklen_t addr_len;
    int index = 0;

	//初始化连接队列
	init_list(pthreadData);
    while(1)
    {
        index = check_list(pthreadData);
        //printf("check----[%d]\n",index);
        if(index >= BACKLOG)
        {
            printf("服务器人数已到上线。。。\n");
            //break;
            continue;
        }
        int ret = accept(sock_fd,(struct sockaddr*)&clie_addr,&addr_len);
        if(ret == -1)
            return -1;

        pthreadData[index].peer.fd = ret;
        pthreadData[index].peer.addr = clie_addr;

        pthread_t pth_id; 
        pthread_attr_t attr ;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
        pthreadData[index].pth_id = pth_id;
        //pthreadData[index].handler = deal_func1;
        pthreadData[index].handler = deal_landing;
        pthread_create(&pth_id,&attr,callBack,&pthreadData[index]);
    }

    return 0;
}

int Close(int sock_fd)
{
    close(sock_fd);
    return 0;
}


int Server_Start(int *sock_fd)
{
    if(sock_fd == NULL)
        goto end;
    int ret = -1;
    int clie_fd;
    *sock_fd = Socket();
    if(*sock_fd == -1)
    {
        printLog(__FILE__,__LINE__,strerror(errno));
        goto end;
    }

    ret = Bind(*sock_fd,PORT);
    if(ret == -1)
    {
        printLog(__FILE__,__LINE__,strerror(errno));
        goto end;
    }

    ret = Listen(*sock_fd,BACKLOG);
    if(ret == -1)
    {
        printLog(__FILE__,__LINE__,strerror(errno));
        goto end;
    }

    return 0;
end:
    printLog(__FILE__,__LINE__,strerror(errno));
    Close(*sock_fd);
    return -1;
}

int Server_End(int sock_fd)
{
    if(sock_fd > 0)
        Close(sock_fd);
    return 0;
}   
