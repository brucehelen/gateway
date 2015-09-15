//
//  mysql_server_helper.c
//  libevent_server
//
//  Created by MissionHealth on 15/9/15.
//  Copyright (c) 2015年 mc. All rights reserved.
//

#include "mysql_server_helper.h"

#include <stdlib.h>
#include <mysql.h>


// mysql
static MYSQL *conn;

void
update_mysql_server(const char *data_string)
{
    char buf[1024];

    // insert into sensor values (0, now(), 'Hello world');
    snprintf(buf, 1024, "insert into sensor values (0, now(), '%s')", data_string);
    if (mysql_query(conn, buf)) {
        printf("insert into sensor table error: %s\n", mysql_error(conn));
        exit(1);
    }
}

/// localhost, root, 142336, gateway
void
connect_to_mysqlserver(const char *server,
                       const char *user,
                       const char *passwd,
                       const char *database)
{
    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, server, user, passwd, database, 0, NULL, 0)) {
        printf("mysql connect error: %s\n", mysql_error(conn));
        exit(1);
    } else {
        printf("Connect mysql server ok\n");
    }
}

