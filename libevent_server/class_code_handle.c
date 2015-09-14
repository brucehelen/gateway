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
#include <stdbool.h>

#include "common.h"
#include "class_code_handle.h"
#include "class_code_define.h"
#include "code_0x40_handle.h"


void
handle_packege_class_code(unsigned char *code, int len, void *ctx)
{
    switch (code[0]) {
        case CLASS_CODE_IP_LIST_UPDATE:
            code_0x40_get_ip_list(code + 1, len - 1, ctx);
            break;

        default:
            break;
    }
}

static bool
is_whole_data_package(unsigned char *data, int len)
{
    if (data[0] == 0x02 && data[1] == 0x55 && data[2] == 0xAA) {
        // get package length
        int package_len = data[3] << 8 | data[4];
        if (package_len == len) {   // 完整的包
            return true;
        }
    }

    return false;
}

void
handle_recv_data(unsigned char *data, int len, void *ctx)
{
    if (is_whole_data_package(data, len)) {
        handle_packege_class_code(&data[5], len - 9, ctx);
    } else {
        printf("data unknown------------\n");
        printf_buf(data, len);
    }
}

