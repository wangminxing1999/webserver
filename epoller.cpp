#include"epoller.h"

bool epoller::addFd(int fd, uint32_t events) {
    struct epoll_event ev = {0};
    ev.events = events;
    ev.data.fd = fd;
    if(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) != 0) {
        return false;
        std::cout<<"epoller add failed "<<fd<<std::endl;
    }
    std::cout<<"epoller add success "<<fd<<std::endl;
    return true;
}

bool epoller::modFd(int fd, uint32_t events) {
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    if(epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1) return false;
    return true;
}

bool epoller::delFd(int fd) {
    if(epoll_ctl(epfd, EPOLL_CTL_DEL, fd, 0) == -1) return false;
    return true;
}

int epoller::wait(int timewait) {
    return epoll_wait(epfd, &active_event[0], MAX_EVENTS, timewait);
}

int epoller::getFd(size_t i) {
    return active_event[i].data.fd;
}

int epoller::getEvents(size_t i) {
    return active_event[i].events;
}