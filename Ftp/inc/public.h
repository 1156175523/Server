#ifndef __PUBLIC_H__
#define __PUBLIC_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <dirent.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define LOG(argc,argv) if(-1 == argc) {\
    printf("[%s|%d]%s:%s\n",__FILE__,__LINE__,argv,strerror(errno));\
    goto end; }

#define DEBUG_LOG(arg) printf("[%s|%d]%s: %s\n",__FILE__,__LINE__,__FUNCTION__,arg);

#endif
