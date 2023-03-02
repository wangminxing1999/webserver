#include"http_conn.h"
std::atomic<int> http_conn::userCount;
std::string http_conn::src_dir = "/home/wmx/桌面/Webserver/resources";

void http_conn::init(int conn_sock_id, sockaddr_in &addr) {
    this->conn_sock_id = conn_sock_id;
    this->client_sockaddr = addr;
    _isClose = false;
    read_buffer.initPtr();
    write_buffer.initPtr();
    userCount++;
}

ssize_t http_conn::readFromSocket(int* saveErrno) {
    ssize_t sum_len = 0;
    while(true) {
        ssize_t len = read_buffer.read(conn_sock_id, saveErrno);
        if (len <= 0) {
            break;
        }
        sum_len += len;
    }
    return sum_len;
}

ssize_t http_conn::writeToSocket(int* saveErrno) {
    ssize_t  sum_len = 0;
    while(true) {
        ssize_t len = writev(conn_sock_id, iov, 2);
        if(len <= 0) {
            *saveErrno = errno;
            break;
        }
        if(iov[0].iov_len == 0 && iov[1].iov_len == 0) {
            break;
        }
        if(static_cast<size_t>(len) > iov[0].iov_len) {
            iov[1].iov_len -= len - iov[0].iov_len;
            iov[1].iov_base = (uint8_t*)iov[1].iov_base + len - iov[0].iov_len;
            if(iov[0].iov_len) {
                iov[0].iov_len = 0;
                write_buffer.initPtr();
            }
        } else {
            iov[0].iov_len -= len;
            iov[0].iov_base = (uint8_t*)iov[0].iov_base + len;
            write_buffer.updateReadIndex(static_cast<int>(len));
        }
        sum_len += len;
    }
    return sum_len;
}

const char *http_conn::getIP() const {
    return inet_ntoa(client_sockaddr.sin_addr);
}

int http_conn::getPort() const {
    return static_cast<int>(ntohs(client_sockaddr.sin_port));
}

int http_conn::getFd() const {
    return conn_sock_id;
}

sockaddr_in http_conn::getAddr() const {
    return client_sockaddr;
}

void http_conn::closeConn() {
    _http_response.unmapFile();
    userCount--;
    _isClose = true;
    std::cout<<"sock closed "<<conn_sock_id<<std::endl;
    close(conn_sock_id);
}

bool http_conn::handle_conn() {
    _http_request.init();
    if(read_buffer.readable_size()<=0) return false;
    if(_http_request.parse(read_buffer)) {
        _http_response.init(src_dir, _http_request.getPath(), _http_request.getMethod(), 0, _http_request.keepAlive());
    } else {
        _http_response.init(src_dir, _http_request.getPath(), _http_request.getMethod(), 400, false);
    }
    _http_response.process(write_buffer);
    iov[0].iov_base = const_cast<char*>(write_buffer.begin_ptr() + write_buffer.getReadIndex());
    iov[0].iov_len = write_buffer.readable_size();
    iov[1].iov_base = _http_response.get_mmapfile();
    iov[1].iov_len = _http_response.get_filestatus().st_size;
    return true;
}