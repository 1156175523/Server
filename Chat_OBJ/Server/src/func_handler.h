#ifndef __FUNC_HANDLER__
#define __FUNC_HANDLER__

//使用到的结构
typedef struct _UserInfo{
    char IP[20];
    char name[256];
    char fd[8];
    //...
}UserInfo;
//使用到的结构

//SIGINT信号处理---处理后台数据
void afterServerClose(int);

//itoa
int itoa(int num,char* out);

//获取用户聊天前缀
int get_prefixInfo(UserInfo* list,int size,const char* IP,const char* fd,char* out_prefix);

//现实提示信息-用户登陆下线
int infoShow(UserInfo* list,int list_size,const char* info);

void* callBack(void* arg);

void* deal_func1(void* arg);

void* deal_landing(void* arg);


#endif
