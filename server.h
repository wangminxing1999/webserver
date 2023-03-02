#ifndef WEBSERVER_SERVER_H
#define WEBSERVER_SERVER_H
#include"buffer.h"
#include"epoller.h"
#include"http_conn.h"
#include"threadpool.h"
#include"timer.h"
#include<arpa/inet.h>
class server {
private:
    bool initSocket();
    void handle_listen();
    void setnonblock(int fd);
    void close_conn(http_conn* client);
    void onRead_(http_conn* client);
    void onWrite_(http_conn* client);
    void onProcess_(http_conn* client);
    void handleRead(http_conn* client);
    void handleWrite(http_conn* client);
    void refreshTime(http_conn* client);
    void addClientConnection(int fd, sockaddr_in addr);
    bool _isClose;
    int _port;
    int _timeout;
    int listenFd;
    std::string src_path;
    std::unique_ptr<threadpool> _threadpool;
    std::unique_ptr<TimeManager> _timemanager;
    std::unique_ptr<epoller> _epoller;
    std::unordered_map<int, http_conn> users;
public:
    server(int port, int timeout, int threadNum):_isClose(false), _port(port),_timeout(timeout), _threadpool(new threadpool(threadNum)),
    _timemanager(new TimeManager()),_epoller(new epoller()) {
        if(!initSocket()) _isClose = true;
        http_conn::userCount = 0;
    }
    ~server() {
        close(listenFd);
        _isClose = true;
    }
    void start();
};
#endif //WEBSERVER_SERVER_H
