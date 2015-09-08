
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sys/socket.h>    
#include <netinet/in.h>    
#include <arpa/inet.h>    
#include <netdb.h>

#include <event.h>

#include <mysql/mysql.h>

using namespace std;

struct event_base* base;

static MYSQL *conn;

void update_mysql_server(char *data_string);
void printf_buf(char *buf, int len);

void onRead(int iCliFd, short iEvent, void *arg)
{
    int iLen;
    char buf[1500];

    iLen = recv(iCliFd, buf, 1500, 0);
	
    if (iLen <= 0) {
        cout << "Client Close" << endl;

        struct event *pEvRead = (struct event*)arg;
        event_del(pEvRead);
        delete pEvRead;

        close(iCliFd);
        return;
    }

    buf[iLen] = 0;
    printf("RECV DATA:\n");
    printf_buf(buf, iLen);
    //update_mysql_server(buf);
}

void printf_buf(char *buf, int len)
{
    for (int i = 0; i < len; i++) {
        if (i % 16 == 0) {
            printf("\n");
        }
        printf("%2X ", buf[i]);
    }
    printf("\n");
}

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

void onAccept(int iSvrFd, short iEvent, void *arg)
{
    int iCliFd;
    struct sockaddr_in sCliAddr;

    socklen_t iSinSize = sizeof(sCliAddr);
    iCliFd = accept(iSvrFd, (struct sockaddr*)&sCliAddr, &iSinSize);
    cout << "ACCEPT" << endl;
    struct event *pEvRead = new event;
    event_set(pEvRead, iCliFd, EV_READ|EV_PERSIST, onRead, pEvRead);
    event_base_set(base, pEvRead);
    event_add(pEvRead, NULL);
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

int main()
{
    int iSvrFd;
    struct sockaddr_in sSvrAddr;

    connect_to_mysqlserver();

    memset(&sSvrAddr, 0, sizeof(sSvrAddr));
    sSvrAddr.sin_family = AF_INET;
    sSvrAddr.sin_port = htons(8081);

    iSvrFd = socket(AF_INET, SOCK_STREAM, 0);
    bind(iSvrFd, (struct sockaddr*)&sSvrAddr, sizeof(sSvrAddr));
    listen(iSvrFd, 10);

    base = event_base_new();

    struct event evListen;

    event_set(&evListen, iSvrFd, EV_READ|EV_PERSIST, onAccept, NULL);
    event_base_set(base, &evListen);
    event_add(&evListen, NULL);
    printf("program start...\n");
    event_base_dispatch(base);

    return 0;
}

