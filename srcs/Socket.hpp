#ifndef WEBSERV_SOCKET_HPP
#define WEBSERV_SOCKET_HPP

#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include "IEventSubscriber.hpp"
#include "Server.hpp"

#define MAX_AWAIT_CONN 900

class Socket : public IEventSubscriber {
private:
    std::vector<Server *>servers;
    int    fd;
public:
    Socket(Server *server);
    int getFd() const;
    std::vector<Server *> getServers() const;
    Server *getDefaultServer() const;
    void appendServer(Server *server);

    class SocketException : public std::exception {
        const char *m_msg;
    public:
        explicit SocketException(const char *msg);

        ~SocketException() throw();

        const char *what() const throw();
    };
};

#endif //WEBSERV_SOCKET_HPP
