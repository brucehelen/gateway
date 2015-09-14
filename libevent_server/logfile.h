//
//  logfile.h
//  libevent_server
//
//  Created by 朱正晶 on 15/9/14.
//  Copyright (c) 2015年 mc. All rights reserved.
//

#ifndef __libevent_server__logfile__
#define __libevent_server__logfile__

#include <stdio.h>
#include <stdint.h>
#include <limits.h>

typedef struct logfile_t {
    char* logfile_name;
    FILE *fp_logfile;
} logfile_t;

typedef enum LogLevel {
    LOG_ERROR,
    LOG_WARN,
    LOG_INFO,
    LOG_DEBUG
} LogLevel;

extern logfile_t *logfile;

int logfile_write(LogLevel level, const char *fmt, ...);
void logfile_close();

#endif /* defined(__libevent_server__logfile__) */
