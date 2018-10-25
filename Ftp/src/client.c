#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "../inc/public.h"
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

#define SAVE_PATH "/home/cxr/Server_cxr/Server/Ftp/bin/"
char get_file[256]={0};
char command[10]  ={0};
int newfd = -1;
int GET=0,LIST=0;


void* read_stdin_callback(void* arg)
{
    int c_fd = *(int*)arg;
    while(1)
    {
        char buf[1024+1] = {0};
        int data_len = 0;
        char head[4] = {0};
        read(STDIN_FILENO,buf,sizeof(buf));
        int index = strlen(buf);
        while(index-1>=0 && buf[index-1]=='\n')
        {
            buf[index-1] = '\0';
            index--;
        }
        index = 0;
        while(buf[index]==' ' || buf[index]=='\n')
        {
            index++;
        }
        strcpy(buf,buf+index);
        //printf("[%s]\n",buf);
        data_len = strlen(buf);
        //printf("data len:%d\n",data_len);
        data_len = htonl(data_len);
        memcpy(head,&data_len,4);
        bzero(command,sizeof(command));
        GET = LIST = 0;

        if(strncmp(buf,"get",3) == 0) 
        {
            char* p = buf+4;
            while(*p == ' ' || *p == '\t')
            {
                p++;
            }
            sprintf(get_file,"%s%s",SAVE_PATH,p);
            DEBUG_LOG(get_file);
            newfd = open(get_file,O_CREAT|O_RDWR,0644);
            if(newfd == -1)
            {perror("OPEN");}
            strcpy(command,"get");
            GET = 1;LIST = 0;
            DEBUG_LOG("GET");
        }
        else if(strncmp(buf,"list",4) == 0)
        {
            strcpy(command,"list");
            LIST = 1;GET = 0;
            DEBUG_LOG("LIST");
        }

        //send(c_fd,head,sizeof(head),0);
        //send(c_fd,buf,strlen(buf),0);
        //合并发送
        char sum[256]={0};
        memcpy(sum,head,4);
        //memcpy(sum,&data_len,4);
        memcpy(sum+4,buf,strlen(buf));
        send(c_fd,sum,4+strlen(buf),0);

#if 0
        int pos = 0;
        while(pos <4+strlen(buf))
        {
            printf("[%c]",sum[pos]);
            pos++;
        }
        printf("\n");
#endif

        //printf("buf:[%s]a\n",buf);
    }
}

void read_in_cback(int c_fd)
{
    char buf[1024+1] = {0};
    int data_len = 0;
    char head[4] = {0};
    read(STDIN_FILENO,buf,sizeof(buf));
    int index = strlen(buf);
    while(index-1>=0 && buf[index-1]=='\n')
    {
        buf[index-1] = '\0';
        index--;
    }
    index = 0;
    while(buf[index]==' ' || buf[index]=='\n')
    {
        index++;
    }
    strcpy(buf,buf+index);
    //printf("[%s]\n",buf);
    data_len = strlen(buf);
    data_len = htonl(data_len);
    memcpy(head,&data_len,4);
    bzero(command,sizeof(command));
    GET = LIST = 0;

    if(strncmp(buf,"get",3) == 0) 
    {
        char* p = buf+4;
        while(*p == ' ' || *p == '\t')
        {
            p++;
        }
        sprintf(get_file,"%s%s",SAVE_PATH,p);
        printf("[%s\n]",get_file);
        newfd = open(get_file,O_CREAT,0644);
        if(newfd == -1)
        {perror("OPEN");}
        strcpy(command,"get");
        GET = 1;LIST = 0;
    }
    else if(strncmp(buf,"list",4) == 0)
    {
        strcpy(command,"list");
        LIST = 1;GET = 0;
    }
    send(c_fd,head,sizeof(head),0);
    send(c_fd,buf,strlen(buf),0);
    //printf("buf:[%s]a\n",buf);
}

//FTP客户端
int ftp_client()
{
    int c_fd = socket(AF_INET,SOCK_STREAM,0);
    LOG(c_fd,"Socket");

    int ret = -1;
    struct sockaddr_in c_addr;
    bzero(&c_addr,sizeof(struct sockaddr));
    socklen_t c_len = sizeof(c_addr);
    c_addr.sin_family      = AF_INET;
    c_addr.sin_port        = htons(20021);
    c_addr.sin_addr.s_addr = INADDR_ANY;

    ret = connect(c_fd,(struct sockaddr*)&c_addr,c_len);
    //printf("%d -- %d\n",ret,errno);
    LOG(ret,"Connect");

    char buf[1024+1]={0};
    char head[4] = {0};
    uint32_t data_len = 0;

    //pthread_t pth;
    //pthread_create(&pth,NULL,read_in_cback,(void*)&c_fd);

    //strcpy(buf,"hello world!");
    //设置非阻塞
    //int flag = fcntl(c_fd,F_GETFL);
    //int tmp = fcntl(c_fd,F_SETFL,flag|O_NONBLOCK);
    //LOG(tmp,"FCNTL");


    //********目前设置非阻塞失败使用多线程处理
    pthread_t pth;
    pthread_create(&pth,NULL,read_stdin_callback,(void*)&c_fd);
    
    while(1)
    {
        //read_in_cback(c_fd);

        int data_len = 0;
        int ret = recv(c_fd,head,4,0);
        if(ret <= 0)
        {
            LOG(ret,"Recv");
            return -1;
        }
        memcpy(&data_len,head,4);
        data_len = ntohl(data_len);
        printf("data_len:[%d]\n",data_len);

        int count = 0;
        do{
            //usleep(500);
            //sleep(2);
            if(GET == 1)
                DEBUG_LOG("GET *******");
            if(LIST == 1)
                DEBUG_LOG("LIST ******");
            int ret = recv(c_fd,buf,1024,0);
            if(ret > 0)
            {
                //printf("\nrecv>>>>>>count[%d]\n",count++);
                //写文件
                if(GET)
                {
                    if(get_file[0] == '\0' || newfd == -1)
                        goto end;
                    write(newfd,buf,ret);
                }
                if(LIST)
                {
                    printf("%s",buf);
                }
                bzero(buf,sizeof(buf));
            }
            else if(ret == 0)
            {
                printf("*****file closed*****\n");
                close(newfd);
                newfd = -1;
            }
            else
            {
                printf("recv < 0:[%d]errno[%d]\n",ret,errno);
                goto end;
            }
            data_len -= 1024;
        }while(data_len > 0);
        printf("%s",buf);
    }
    
    pthread_join(pth,NULL);

    if(newfd != -1)
        close(newfd);
    return 0;
end:
    if(newfd != -1)
        close(newfd);
    return -1;
}


int main(int argc,char* argv[])
{
    ftp_client();
    return 0;
}
