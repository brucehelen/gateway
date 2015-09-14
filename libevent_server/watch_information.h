//
//  watch_information.h
//  libevent_server
//
//  Created by 朱正晶 on 15/9/11.
//  Copyright (c) 2015年 mc. All rights reserved.
//

#ifndef libevent_server_watch_information_h
#define libevent_server_watch_information_h

#include <sys/socket.h>
#include <netinet/in.h>
#include <event.h>

struct watch_information {
    struct sockaddr_in in_address;      // 手表的IP地址
    struct bufferevent *bev;            // socket buffer
};

typedef struct watch_information watch_information;

#endif
