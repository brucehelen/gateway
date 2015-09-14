//
//  common.c
//  libevent_server
//
//  Created by MissionHealth on 15/9/14.
//  Copyright (c) 2015年 mc. All rights reserved.
//

#include "common.h"
#include <string.h>
#include <stdlib.h>
#include "watch_information.h"

void
printf_buf(unsigned char *buf, ssize_t len)
{
    for (ssize_t i = 0; i < len; i++) {
        if (i % 16 == 0) {
            printf("\n");
        }
        printf("%2X ", buf[i]);
    }
    printf("\n");
}

// BCD转十进制
int
BCDToInt(unsigned char bcd)
{
    return (0xff & (bcd>>4))*10 +(0xf & bcd);
}

/**
 *  服务器端向客户端发送数据，自动加入包头和包尾
 *
 *  @param code 类别代码
 *  @param data 待发送数据buf首地址
 *  @param len  待发送数据长度
 *  @param ctx  watch_information
 */
void
server_to_client_cmd(unsigned char code, void *data, int len, void *ctx)
{
    int index = 0;
    int send_length = len + 9;
    unsigned char *send_buf = (unsigned char*)malloc(send_length);
    if (send_buf == NULL ) {
        printf("low memory\n");
        return;
    }

    // header
    send_buf[index++] = 0x02;
    send_buf[index++] = 0x55;
    send_buf[index++] = 0xAA;

    // len
    send_buf[index++] = (send_length >> 8) & 0xFF;
    send_buf[index++] = (send_length) & 0xFF;

    // class code
    send_buf[index++] = code;

    // data
    memcpy(send_buf + index, data, len);

    // tail
    index += len;
    send_buf[index++] = 0xAA;
    send_buf[index++] = 0x55;
    send_buf[index++] = 0x03;

    watch_information *info = (watch_information *)ctx;

    struct evbuffer *output = bufferevent_get_output(info->bev);

    if (evbuffer_add(output, send_buf, send_length) == -1) {
        printf("send data error\n");
    }

    // free memory
    free(send_buf);
}

