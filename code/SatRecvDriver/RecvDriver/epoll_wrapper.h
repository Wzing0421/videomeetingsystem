#ifndef EPOLL_H_
#define EPOLL_H_
#ifdef PTHREAD
#include<sys/epoll.h>

class Epoll{
public:
    Epoll(int timeout = 200/*default wait time, 200ms*/);
    ~Epoll();
    bool AddEvent(int fd, int events);
    bool DelEvent(int fd, int events);
    int Wait();
    void SetTimeOut(int timeout);

    const epoll_event* ReadyEvents() const{
        return events_;
    }
private:
    enum{
        MAX_EVENT_NUM = 10  //max number of epoll event
    };

    int epoll_fd_;
    int time_out_;
    struct epoll_event events_[MAX_EVENT_NUM];
};

#endif

#endif
