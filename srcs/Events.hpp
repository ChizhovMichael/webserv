#ifndef WEBSERV_EVENTS_HPP
#define WEBSERV_EVENTS_HPP

#include <iostream>
#include <set>
#include <sys/event.h>
#include <cerrno>

class Events {
private:
    const int     max_size;
    int           queue_fd;
    std::set<int> fds;
    struct kevent *w_event;
    struct kevent *res_event;
public:
    Events(int max_size);
    void subscribe(int fd, short type);
    void unsubscribe(int fd);
    std::pair<int, struct kevent *> getUpdates(int = 5);
    std::set<int> &getFds();

    class KqueueException : public std::exception {
        virtual const char *what() const throw();
    };
};


#endif //WEBSERV_EVENTS_HPP
