#ifndef WEBSERVER_EPOLLER_H
#define WEBSERVER_EPOLLER_H
#include<sys/epoll.h>
#include<vector>
#include<fcntl.h>
#include<unistd.h>
#include<iostream>
#define MAX_EVENTS 1024
class epoller{
private:
    int epfd;
    std::vector<struct epoll_event> active_event;
public:
    epoller() {
        epfd = epoll_create(5);
        active_event = std::vector<struct epoll_event>(MAX_EVENTS);
    }
    ~epoller() {
        close(epfd);
    }
    bool addFd(int fd, uint32_t events);
    bool modFd(int fd, uint32_t events);
    bool delFd(int fd);
    int wait(int timewait = -1);
    int getFd(size_t i);
    int getEvents(size_t i);
};
#endif //WEBSERVER_EPOLLER_H
