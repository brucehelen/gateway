//
//  common.h
//  libevent_server
//
//  Created by MissionHealth on 15/9/14.
//  Copyright (c) 2015年 mc. All rights reserved.
//

/// 常用的方法集合

#ifndef __libevent_server__common__
#define __libevent_server__common__

#include <stdio.h>

/**
 *  十六进制打印buffer信息
 */
void printf_buf(unsigned char *buf, ssize_t len);

// BCD转十进制
int BCDToInt(unsigned char bcd);

/**
 *  服务器端向客户端发送数据，自动加入包头和包尾
 *
 *  @param code 类别代码
 *  @param data 待发送数据buf首地址
 *  @param len  待发送数据长度
 *  @param ctx  watch_information 结构体
 */
void server_to_client_cmd(unsigned char code, void *data, int len, void *ctx);

#endif /* defined(__libevent_server__common__) */

