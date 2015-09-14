//
//  logfile.c
//  libevent_server
//
//  Created by 朱正晶 on 15/9/14.
//  Copyright (c) 2015年 mc. All rights reserved.
//

#include "logfile.h"

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#define LOG_BUF_SIZE        1024

logfile_t *logfile;

int
logfile_write(LogLevel level, const char *fmt, ...)
{
    if (logfile == NULL) return 1;

    // open file
    if (logfile->fp_logfile == NULL) {
        logfile->fp_logfile = fopen(logfile->logfile_name, "a+");

        if (NULL == logfile->fp_logfile) {
            fprintf(stderr, "Can't open logfile %s", logfile->logfile_name);
            exit(EXIT_FAILURE);
        }
    }

    time_t now;
    time(&now);
    char log_buf[LOG_BUF_SIZE];
    strftime(log_buf, 20, "%Y/%m/%d %X", localtime(&now));
    int info_len = 19;

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(log_buf + 19, LOG_BUF_SIZE - 20, fmt, ap);
    va_end(ap);

    fprintf(logfile->fp_logfile, "%s\n", log_buf);
    fflush(logfile->fp_logfile);

    return 0;
}

void
logfile_close(void)
{
    if (logfile->fp_logfile) {
        fclose(logfile->fp_logfile);
        logfile->fp_logfile = NULL;
    }
}
