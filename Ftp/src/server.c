#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "../inc/public.h"

//ftp服务器
#define FTP_PORT 20021
#define MAX_SENDSIZE 1024
#define FTP_SERVER_ADDR "127.0.0.1"
#define BACK_LOG 256            //监听个数
#define FILE_DIR "/home/cxr/FTP/"
int s_fd = -1;  //服务器文件描述符

int send_list(int c_fd,char* dir)
{
    char head[4] = {0};
    char data[MAX_SENDSIZE+1] = {0};
    DIR* d = opendir(dir);
    if(d == NULL){perror("Opendir");}
    struct dirent* entity = NULL;
    while((entity = readdir(d)) != NULL)
    {
        if(entity->d_type == DT_REG)
        {
            sprintf(data,"%s%s%s",data,entity->d_name,"\n");
        }
        //溢出处理
    }
    //printf("file:>>>>>>\n");
    //printf("%s\n",data);
    //printf("file:>>>>>>\n");
    //data[strlen(data)-1] = '\0';
    int data_len = strlen(data);
    data_len = htonl(data_len);
    memcpy(head,&data_len,4);
    send(c_fd,head,4,0);
    send(c_fd,data,strlen(data),0);
    
    return 0;
end:
    return -1;
}

int send_file(int c_fd,char* filename)
{
    if(filename == NULL || filename[0] == '\0')
    {
        printf("文件名为空\n");
        goto end;
    }
    char file[1024] = {0};
    char data_buf[MAX_SENDSIZE+1] = {0};
    sprintf(file,"%s%s",FILE_DIR,filename);
    printf("real_file：[%s]\n",file);
    if(access(file,F_OK) != 0)
    {
        printf("文件不存在！\n");
        goto end;
    }
    
    int fd = open(file,O_RDONLY);
    if(fd <= 0) 
    {
        printf("[%s|%d]%s:%s\n",__FILE__,__LINE__,__FUNCTION__,strerror(errno));
        goto end;
    }

    uint32_t data_len = htonl(lseek(fd,0,SEEK_END) - lseek(fd,0,SEEK_SET));
    printf("file szie:%d\n",(int)(lseek(fd,0,SEEK_END) - lseek(fd,0,SEEK_SET)));
    char head[4] = {0};
    memcpy(head,&data_len,4);
    send(c_fd,head,4,0);

    int count = 0;
    int ret = 0;

    while((ret = read(fd,data_buf,1024)) > 0)
    {
        //printf(">>send>> %d data_buf[%d]>>>>>\n",++count,ret);
        printf("%s",data_buf);
        int s_ret = send(c_fd,data_buf,ret,0);
        //printf("@@@@@@@@@@@@@@@@@@@-->s_ret :%d -- errno:%d\n",s_ret,errno);
        if(s_ret < 0) {perror("Send");}
        memset(data_buf,0x00,sizeof(data_buf));
    }

    close(fd);
    return 0;
end:
    close(fd);
    return -1;
}

int parse_command(int c_fd,char* data)
{
    //uint32_t data_len = 0;
    //memcpy(&data_len,data,4);
    //data_len = ntohl(data_len);
    //printf("data_len:%d \n",data_len);
    //sprintf(data,"%s",data+4);
    printf("commad:[%s]\n",data);

#if 0
    int index = 0;
    while(index < 120)
    {
        printf("[%c]",data[index]);
        index++;
    }
    printf("\n");
#endif

    if(strncmp("get",data,3) == 0) 
    {
        //get 
        char filename[256] = {0};
        strcpy(filename,data+4);
        int index = strlen(filename)-1;
        while(index >= 0 && filename[index] == '\n')
        {
            filename[index--] = '\0';
        }
        printf(">>>>>>>get请求 file:[%s]\n",filename);
        int ret = send_file(c_fd,filename);
        if(ret == -1 ) goto end;
    }
    else if(strncmp("list",data,4) == 0)
    {
        //list
        printf(">>>>>>>list请求\n");
        int ret = send_list(c_fd,FILE_DIR);
        if(ret == -1) goto end;
    }
    else
    {
        char mesg[256] = "无效请求";
        printf("%s-->[%s]\n",mesg,data);
        //goto end;
    }
    
    return 0;
end:
    return -1;
}

int ftp_server()
{
    int ret = -1;
    struct sockaddr_in s_addr;
    bzero(&s_addr,sizeof(struct sockaddr_in));
    s_addr.sin_family      = AF_INET;
    s_addr.sin_port        = htons(FTP_PORT);
    s_addr.sin_addr.s_addr = inet_addr(FTP_SERVER_ADDR);


    s_fd = socket(AF_INET,SOCK_STREAM,0);
    LOG(s_fd,"Socket");
    
    //printf("SERVER FD:%d******\n",s_fd);
    //端口复用
    int l=1;
    ret = setsockopt(s_fd,SOL_SOCKET,SO_REUSEADDR,(char *)&l,sizeof(l));
    LOG(ret,"Setsockopt");

    socklen_t len = sizeof(struct sockaddr);
    ret = bind(s_fd,(struct sockaddr*)&s_addr,len); 
    LOG(ret,"Bind");


    ret = listen(s_fd,BACK_LOG);
    LOG(ret,"Listen");

    int c_fd = -1;
    struct sockaddr_in c_addr;
    socklen_t c_len;
    bzero(&c_addr,sizeof(struct sockaddr_in));
    bzero(&c_len,sizeof(socklen_t));


loop:
    c_fd = accept(s_fd,(struct sockaddr*)&c_addr,&c_len);
    LOG(c_fd,"Accept");
    char buf[MAX_SENDSIZE+1] = {0};
    while(1)
    {
        ret = recv(c_fd,buf,sizeof(buf),0);
        if(ret > 0)
        {
            ret = parse_command(c_fd,buf);
            if(ret < 0)
            {
                goto end;
            }
            memset(buf,0x00,sizeof(buf));
        }
        else if(!ret)
        {
            close(c_fd);
            goto loop;
        }
        else
        {
            break;
        }
        bzero(buf,sizeof(buf));
    }

    return 0;
end:
    return -1;
}

int main(int argc,char* argv[])
{
    int ret = ftp_server();
    if(ret == -1)
        return -1;
    return 0;
}
