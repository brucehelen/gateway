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
#include <syslog.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <signal.h>

#include <event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include "class_code_handle.h"
#include "watch_information.h"


#define LISTEN_PORT     8081
#define LISTEN_BACKLOG  50
// 接收到数据，分配在栈中的内存大小
#define RECV_BUF_SIZE   512

#define handle_error(msg) \
        do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define handle_infomation(msg) \
        do { printf(msg); } while(0)

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
        printf("%s:%d disconnect",
               inet_ntoa(in_address->sin_addr),
               in_address->sin_port);

        syslog(LOG_INFO, "%s:%d disconnect\n",
               inet_ntoa(in_address->sin_addr),
               in_address->sin_port);

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

    // malloc new memory, don't forget to free
    watch_information *watch_info = (watch_information *)malloc(sizeof(watch_information));
    if (bev == NULL || watch_info == NULL) {
        handle_infomation("memory low error\n");
        return;
    }

    memcpy(&watch_info->in_address, address, sizeof(struct sockaddr_in));
    watch_info->bev = bev;

    syslog(LOG_INFO, "%s:%d Connect\n",
           inet_ntoa(watch_info->in_address.sin_addr),
           watch_info->in_address.sin_port);

    bufferevent_setcb(bev, recv_read_cb, NULL, event_cb, (void *)watch_info);
    bufferevent_enable(bev, EV_READ|EV_WRITE);
}

static void
accept_error_cb(struct evconnlistener *listener, void *ctx)
{
    struct event_base *base = evconnlistener_get_base(listener);
    int err = EVUTIL_SOCKET_ERROR();
    syslog(LOG_ERR,
           "Got an error %d (%s) on the listener. Shutting down",
           err,
           evutil_socket_error_to_string(err));

    event_base_loopexit(base, NULL);
}

static void
show_program_start_info(struct event_base *base)
{
    syslog(LOG_INFO, "---- Program start ----");
    syslog(LOG_INFO, "Using Libevent with backend method %s",
           event_base_get_method(base));
}

static void
daemonize(const char *cmd)
{
    int                 i, fd0, fd1, fd2;
    pid_t               pid;
    struct rlimit       rl;
    struct sigaction    sa;

    umask(0);

    if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
        handle_error("can't get file limit");
    }

    // Become a session leader to lose controlling TTY.
    if ((pid = fork()) < 0) {
        handle_error("fork error");
    } else if (pid != 0) {      // parent
        exit(0);
    }
    setsid();

    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) < 0) {
        handle_error("can't ignore SIGHUP");
    }
    if ((pid = fork()) < 0) {
        handle_error("can't fork");
    } else if (pid != 0)
        exit(0);

    if (chdir("/") < 0) {
        handle_error("can't change directory to /");
    }

    // close all open file descriptors
    if (rl.rlim_max == RLIM_INFINITY)
        rl.rlim_max = 1024;
    for (i = 0; i < rl.rlim_max; i++)
        close(i);

    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);

    openlog(cmd, LOG_CONS, LOG_DAEMON);
    if (fd0 != 0 || fd1 != 1 || fd2 != 2) {
        syslog(LOG_ERR, "unexpected file descriptors %d, %d, %d", fd0, fd1, fd2);
        exit(1);
    }
}

int
main(int argc, char **argv)
{
    struct event_base *base;
    struct evconnlistener *listener;
    struct sockaddr_in sin;

    // 调试时可以先不以守护进行的方式运行
    daemonize("gateway");

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
    if (!listener) syslog(LOG_ERR, "Couldn't create evconnlistener");

    evconnlistener_set_error_cb(listener, accept_error_cb);

    show_program_start_info(base);
    event_base_dispatch(base);

    evconnlistener_free(listener);
    event_base_free(base);

    syslog(LOG_ERR, "---- exit ----");

    return 0;
}


