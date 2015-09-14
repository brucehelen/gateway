//
//  code_0x40_handle.c
//  libevent_server
//
//  Created by MissionHealth on 15/9/14.
//  Copyright (c) 2015年 mc. All rights reserved.
//

#include "code_0x40_handle.h"
#include "common.h"
#include "watch_information.h"
#include "class_code_define.h"


/**
 *  将IP地址和端口按照要求转换为字符串
 *  210.61.64.60:5151  ---> "21006106406005151"
 *
 *  @param address IP地址
 *  @param out     转换完成的字符串(Buffer最小为18bytes)
 */
void
convert_ip4_int_to_string(struct sockaddr_in *address, char *out)
{
    in_addr_t host_ip = ntohl(address->sin_addr.s_addr);
    for (int i = 3; i >= 0; i--) {
        int seg = (host_ip >> (8 * i)) & 0xFF;
        sprintf(out + 9 - i*3, "%.3d", seg);
    }

    sprintf(out + 12, "%.4d", address->sin_port);
}


void
code_0x40_get_ip_list(unsigned char *code, int len, void *ctx)
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
