#ifdef PTHREAD
#include<stdio.h>
#include<assert.h>
#include"epoll_wrapper.h"
Epoll::Epoll(int timeout):
time_out_(timeout){
    if ((epoll_fd_ = epoll_create(MAX_EVENT_NUM)) < 0){
        perror("epoll create!");
    }
}

Epoll::~Epoll(){

}

bool Epoll::AddEvent(int fd, int events){
    assert(epoll_fd_ > 0);

    epoll_event event;
    event.data.fd = fd;
    event.events = events;
    return epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event) == 0;
}

bool Epoll::DelEvent(int fd, int events){
    assert(epoll_fd_ > 0);

    epoll_event event;
    event.data.fd = fd;
    event.events = events;
    return epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, &event) == 0;
}

void Epoll::SetTimeOut(int out_time){
    time_out_ = out_time;
}

int Epoll::Wait(){
    int ready_event_num = epoll_wait(epoll_fd_, events_, MAX_EVENT_NUM, time_out_);
    return ready_event_num;
}
#endif
