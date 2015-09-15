//
//  mysql_server_helper.h
//  libevent_server
//
//  Created by MissionHealth on 15/9/15.
//  Copyright (c) 2015年 mc. All rights reserved.
//

#ifndef __libevent_server__mysql_server_helper__
#define __libevent_server__mysql_server_helper__

#include <stdio.h>

void
update_mysql_server(const char *data_string);

void
connect_to_mysqlserver(const char *server,
                       const char *user,
                       const char *passwd,
                       const char *database);

#endif /* defined(__libevent_server__mysql_server_helper__) */
