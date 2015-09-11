//
//  class_code_handle.c
//  libevent_server
//
//  Created by 朱正晶 on 15/9/10.
//  Copyright (c) 2015年 mc. All rights reserved.
//

//  处理类别代码

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "class_code_handle.h"
#include "class_code_define.h"
#include "watch_information.h"

void printf_buf(unsigned char *buf, ssize_t len)
{
    for (ssize_t i = 0; i < len; i++) {
        if (i % 16 == 0) {
            printf("\n");
        }
        printf("%2X ", buf[i]);
    }
    printf("\n");
}

//BCD转十进制
int BCDToInt(unsigned char bcd)
{
    return (0xff & (bcd>>4))*10 +(0xf & bcd);
}

/**
 *  服务器端向客户端发送数据，会加入包头和包尾
 *
 *  @param code 类别代码
 *  @param data 待发送数据buf首地址
 *  @param len  待发送数据长度
 *  @param ctx  watch_information
 */
void server_to_client_cmd(unsigned char code, void *data, int len, void *ctx)
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

/**
 *  将IP地址和端口按照要求转换为字符串
 *  210.61.64.60:5151  ---> "21006106406005151"
 *
 *  @param address IP地址
 *  @param out     转换完成的字符串(Buffer最小为18bytes)
 */
void convert_ip4_int_to_string(struct sockaddr_in *address, char *out)
{
    in_addr_t host_ip = ntohl(address->sin_addr.s_addr);
    for (int i = 3; i >= 0; i--) {
        int seg = (host_ip >> (8 * i)) & 0xFF;
        sprintf(out + 9 - i*3, "%.3d", seg);
    }

    sprintf(out + 12, "%.4d", address->sin_port);
}

void get_ip_list(unsigned char *code, int len, void *ctx)
{
    char imei[16], imsi[16];

    for (int i = 0; i < 7; i++) {
        snprintf(imei + i*2, 16 - 2*i, "%.2d", BCDToInt(code[i]));
        snprintf(imsi + i*2, 16 - 2*i, "%.2d", BCDToInt(code[i + 8]));
    }

    sprintf(imei + 14, "%d", code[7] >> 4 & 0x0F);
    sprintf(imsi + 14, "%d", code[15] >> 4 & 0x0F);

    printf("imei = %s, imsi = %s\n", imei, imsi);

    // do something...

    // send back to client
    char ip_buf[18];
    watch_information *info = (watch_information *)ctx;
    convert_ip4_int_to_string(&info->in_address, ip_buf);
    printf("IP address: %s\n", ip_buf);
    server_to_client_cmd(CLASS_CODE_IP_LIST_UPDATE, ip_buf, 17, ctx);
}

void handle_packege_class_code(unsigned char *code, int len, void *ctx)
{
    switch (code[0]) {
        case CLASS_CODE_IP_LIST_UPDATE:
            get_ip_list(code + 1, len, ctx);
            break;
            
        default:
            break;
    }
}

void handle_recv_data(unsigned char *data, int len, void *ctx)
{
    if (data[0] == 0x02 && data[1] == 0x55 && data[2] == 0xAA) {
        int package_len = data[3] << 8 | data[4];
        if (package_len == len) {   // 完整的包
            handle_packege_class_code(&data[5], len - 9, ctx);
        } else {                    // 不完整，需要存储，最后拼接
            // TODO:
        }
    } else {
        printf("data unknown------------\n");
        printf_buf(data, len);
    }
}

