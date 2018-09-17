#include <stdio.h>
#include <signal.h>
#include "server_tcp.h"
#include "func_handler.h"
#include "mmysql.h"

int sock_fd = -1;

int main()
{
    signal(SIGINT,afterServerClose);

    Server_Start(&sock_fd);
    printf("Chat Server Start!\n");

    //处理
    Accept(sock_fd);

    Server_End(sock_fd);

    return 0;
}

