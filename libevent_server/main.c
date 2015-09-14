//
//  main.c
//  libevent_server
//
//  Created by 朱正晶 on 15/9/9.
//  Copyright (c) 2015年 mc. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#include <mysql.h>
#include <event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include "class_code_handle.h"
#include "watch_information.h"

#include <syslog.h>

// mysql
static MYSQL *conn;
// log file
logfile_t g_logfile;

#define LISTEN_PORT     8081
#define LISTEN_BACKLOG  50
// 分配在栈中的内存
#define RECV_BUF_SIZE   512

#define LOG_FILE_PATH   "/tmp/gateway.log"

#define handle_error(msg) \
        do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define handle_infomation(msg) \
        do { printf(msg); } while(0)


void update_mysql_server(char *data_string)
{
    char buf[1024];
    
    // insert into sensor values (0, now(), 'Hello world');
    snprintf(buf, 1024, "insert into sensor values (0, now(), '%s')", data_string);
    if (mysql_query(conn, buf)) {
        printf("insert into sensor table error: %s\n", mysql_error(conn));
        exit(1);
    }
}

void connect_to_mysqlserver()
{
    char server[] = "localhost";
    char user[] = "root";
    char password[] = "142336";
    char database[] = "gateway";

    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0)) {
        printf("mysql connect error: %s\n", mysql_error(conn));
        exit(1);
    } else {
        printf("Connect mysql server ok\n");
    }
}

/**
 *  接收数据事件
 *
 *  @param bev bufferevent
 *  @param ctx 附加信息: IP地址信息
 */
static void
recv_read_cb(struct bufferevent *bev, void *ctx)
{
    unsigned char recv_buf[RECV_BUF_SIZE];

    /* This callback is invoked when there is data to read on bev. */
    struct evbuffer *input = bufferevent_get_input(bev);
    //struct evbuffer *output = bufferevent_get_output(bev);

    size_t buf_len = evbuffer_get_length(input);
    if (buf_len > 0) {
        if (buf_len > RECV_BUF_SIZE) {
            unsigned char *big_buf = malloc(buf_len);
            int len = evbuffer_remove(input, (void *)big_buf, buf_len);
            handle_recv_data(big_buf, len, ctx);
            free(big_buf);
        } else {
            int len = evbuffer_remove(input, (void *)recv_buf, RECV_BUF_SIZE);
            handle_recv_data(recv_buf, len, ctx);
        }
    }

    /* Copy all the data from the input buffer to the output buffer. */
    //evbuffer_add_buffer(output, input);
}

/**
 *  是否发送完成
 *
 *  @param bev bufferevent
 *  @param ctx 附加信息
 */
static void
write_cb(struct bufferevent *bev, void *ctx)
{
    
}

static void
event_cb(struct bufferevent *bev, short events, void *ctx)
{
    struct sockaddr_in *in_address = (struct sockaddr_in *)ctx;
    printf("echo_event_cb events = 0x%x\n", events);

    if (events & BEV_EVENT_ERROR)
        bufferevent_free(bev);

    // disconnect
    if (events & (BEV_EVENT_READING | BEV_EVENT_EOF)) {
        printf("%s:%d disconnect\n", inet_ntoa(in_address->sin_addr), in_address->sin_port);
        free(ctx);
    }
}

/**
 *  We got a new connection! Set up a bufferevent for it.
 *
 *  @param listener The evconnlistener
 *  @param fd       The new file descriptor
 *  @param address  The source address of the connection
 *  @param socklen  The length of addr
 *  @param ctx      the pointer passed to evconnlistener_new()
 */
static void
accept_conn_cb(struct evconnlistener *listener,
               evutil_socket_t fd,
               struct sockaddr *address,
               int socklen,
               void *ctx)
{
    struct event_base *base = evconnlistener_get_base(listener);
    struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    watch_information *watch_info = (watch_information *)malloc(sizeof(watch_information));
    if (bev == NULL || watch_info == NULL) {
        handle_infomation("memory low error\n");
        return;
    }

    memcpy(&watch_info->in_address, address, sizeof(struct sockaddr_in));
    watch_info->bev = bev;
    printf("%s:%d Connect\n",
           inet_ntoa(watch_info->in_address.sin_addr),
           watch_info->in_address.sin_port);

    bufferevent_setcb(bev, recv_read_cb, write_cb, event_cb, (void *)watch_info);
    bufferevent_enable(bev, EV_READ|EV_WRITE);
}

static void
accept_error_cb(struct evconnlistener *listener, void *ctx)
{
    struct event_base *base = evconnlistener_get_base(listener);
    int err = EVUTIL_SOCKET_ERROR();
    fprintf(stderr, "Got an error %d (%s) on the listener. "
            "Shutting down.\n", err, evutil_socket_error_to_string(err));

    event_base_loopexit(base, NULL);
}

static void
show_program_start_info(struct event_base *base)
{
    printf("\n---- Program start ----\n");

    syslog(LOG_USER|LOG_INFO, "syslog test message generated in program\n");

    printf("Using Libevent with backend method %s.\n",
           event_base_get_method(base));
}

int
main(int argc, char **argv)
{
    struct event_base *base;
    struct evconnlistener *listener;
    struct sockaddr_in sin;

    // log file
    g_logfile.logfile_name = LOG_FILE_PATH;

    base = event_base_new();
    if (!base) handle_error("Couldn't open event base");

    /* Clear the sockaddr before using it, in case there are extra
     * platform-specific fields that can mess us up. */
    memset(&sin, 0, sizeof(sin));
    /* This is an INET address */
    sin.sin_family = AF_INET;
    /* Listen on 0.0.0.0 */
    sin.sin_addr.s_addr = htonl(0);
    /* Listen on the given port. */
    sin.sin_port = htons(LISTEN_PORT);

    listener = evconnlistener_new_bind(base, accept_conn_cb, NULL,
                                       LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, -1,
                                       (struct sockaddr*)&sin, sizeof(sin));
    if (!listener) handle_error("Couldn't create evconnlistener");

    evconnlistener_set_error_cb(listener, accept_error_cb);

    show_program_start_info(base);
    event_base_dispatch(base);

    evconnlistener_free(listener);
    event_base_free(base);

    return 0;
}


