#include "Events.hpp"

Events::Events(int max_size) : max_size(max_size) {
    this->queue_fd = kqueue();
    if (this->queue_fd == -1)
        throw KqueueException();
    this->w_event   = new struct kevent;
    this->res_event = new struct kevent[this->max_size];
}

void Events::subscribe(int fd, short type) {
    this->fds.insert(fd);

    uint64_t t_fd   = fd;
    uint32_t fflags = 0;
    EV_SET(this->w_event, t_fd, type, EV_ADD, fflags, 0, NULL);
    if (kevent(this->queue_fd, this->w_event, 1, nullptr, 0, nullptr) == -1) {
        std::cout << strerror(errno) << std::endl;
        throw KqueueException();
    }
    if (type == EVFILT_WRITE) {
        if (kevent(this->queue_fd, this->w_event, 1, nullptr, 0, nullptr) == -1) {
            std::cout << strerror(errno) << std::endl;
            throw KqueueException();
        }
    }
}

void Events::unsubscribe(int fd) {
    this->fds.erase(fd);
}

std::pair<int, struct kevent *> Events::getUpdates(int tout) {
    struct timespec tmout = {tout,0};
    int             res   = kevent(this->queue_fd, nullptr, 0, this->res_event, this->max_size, &tmout);

    return std::make_pair(res, this->res_event);
}

std::set<int> &Events::getFds() {
    return this->fds;
}

const char *Events::KqueueException::what() const throw() {
    return exception::what();
}
