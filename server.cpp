#include"server.h"

bool server::initSocket() {
    int ret;
    struct sockaddr_in addr;
    addr.sin_port = htons(_port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_family = AF_INET;
    std::cout<<inet_ntoa(addr.sin_addr)<<std::endl;
    listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd < 0) return false;
    int optval = 1;
    ret = setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
    if(ret < 0) {
        close(listenFd);
        return false;
    }
    ret = bind(listenFd, (struct sockaddr *)&addr, sizeof(addr));
    if(ret < 0) {
        close(listenFd);
        return false;
    }
    ret = listen(listenFd, 6);
    if(ret < 0) {
        close(listenFd);
        return false;
    }
    ret = _epoller->addFd(listenFd, EPOLLIN | EPOLLET);
    if(ret == 0) {
        close(listenFd);
        return false;
    }
    setnonblock(listenFd);
    return true;
}

void server::setnonblock(int fd) {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}

void server::onRead_(http_conn *client) {
    int readErrno = 0;
    ssize_t res = client->readFromSocket(&readErrno);
    std::cout<<"read_in "<<res<<std::endl;
    if(res <= 0 && readErrno != EAGAIN) {
        client->closeConn();
        return;
    }
    onProcess_(client);
}

void server::onWrite_(http_conn *client) {
    int writeErrno = 0;
    ssize_t res = client->writeToSocket(&writeErrno);
    if(client->writebytes() == 0) {
        if(client->keepalive()) {
            onProcess_(client);
            return;
        }
    } else if(res < 0) {
        if(writeErrno == EAGAIN) {
            _epoller->modFd(client->getFd(), EPOLLOUT | EPOLLET);
            return;
        }
    }
    client->closeConn();
}

void server::onProcess_(http_conn *client) {
    if(client->handle_conn()) {
        _epoller->modFd(client->getFd(), EPOLLOUT | EPOLLET);
    } else {
        _epoller->modFd(client->getFd(), EPOLLIN | EPOLLET);
    }
}

void server::refreshTime(http_conn *client) {
    _timemanager->update(client->getFd(), _timeout);
}

void server::handleRead(http_conn *client) {
    refreshTime(client);
    _threadpool->commit(std::bind(&server::onRead_, this, client));
}

void server::handleWrite(http_conn *client) {
    refreshTime(client);
    _threadpool->commit(std::bind(&server::onWrite_, this, client));
}

void server::close_conn(http_conn *client) {
    _epoller->delFd(client->getFd());
    client->closeConn();
}

void server::addClientConnection(int fd, sockaddr_in addr) {
    users[fd].init(fd, addr);
    _timemanager->add_node(fd, _timeout, std::bind(&server::close_conn,this,&users[fd]));
    _epoller->addFd(fd, EPOLLIN | EPOLLET | EPOLLONESHOT | EPOLLRDHUP);
    setnonblock(fd);
}

void server::handle_listen() {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    while(true) {
        int fd = accept(listenFd, (struct sockaddr*)&addr, &len);
        if(fd <= 0) return;
        addClientConnection(fd, addr);
    }
}

void server::start() {
    int timeMS = -1;
    while(!_isClose) {
        timeMS = _timemanager->get_next_handle();
        std::cout<<"next faded out "<<timeMS<<std::endl;
        int eventCnt = _epoller->wait(timeMS);
        std::cout<<"event_number "<<eventCnt<<std::endl;
        for(int i = 0; i < eventCnt; i++) {
            int fd = _epoller->getFd(i);
            std::cout<<fd<<std::endl;
            uint32_t events = _epoller->getEvents(i);
            if(fd == listenFd) {
                std::cout<<"handle listen"<<std::endl;
                handle_listen();
            } else if(events & EPOLLIN) {
                std::cout<<"handle in"<<std::endl;
                handleRead(&users[fd]);
            } else if(events & EPOLLOUT) {
                std::cout<<"handle out"<<std::endl;
                handleWrite(&users[fd]);
            } else if(events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                std::cout<<"close"<<std::endl;
                close_conn(&users[fd]);
            } else {
                std::cout<<"unexpected events happen!\n"<<std::endl;
            }
        }
    }
}