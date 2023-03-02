#ifndef WEBSERVER_HTTP_CONN_H
#define WEBSERVER_HTTP_CONN_H
#include"buffer.h"
#include"http_request.h"
#include"http_response.h"
#include<atomic>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
class http_conn {
public:
    http_conn(){}
    ~http_conn(){closeConn();}
    void init(int conn_sock_id, sockaddr_in& addr);
    ssize_t readFromSocket(int* saveErrno);
    ssize_t writeToSocket(int* saveErrno);
    const char* getIP() const;
    int getPort() const;
    int getFd() const;
    int writebytes() {
        return iov[0].iov_len + iov[1].iov_len;
    }
    bool keepalive() {
        return _http_request.keepAlive();
    }
    sockaddr_in getAddr() const;
    bool handle_conn();
    void closeConn();
    static std::atomic<int> userCount;
    static std::string src_dir;
private:
    int conn_sock_id;
    bool _isClose;
    struct sockaddr_in client_sockaddr;
    struct iovec iov[2];
    buffer read_buffer;
    buffer write_buffer;
    http_request _http_request;
    http_response _http_response;
};


#endif //WEBSERVER_HTTP_CONN_H
