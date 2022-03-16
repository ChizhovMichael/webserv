#ifndef WEBSERV_DAEMON_HPP
#define WEBSERV_DAEMON_HPP

#include <iostream>
#include <map>
#include <set>
#include <csignal>
#include "Config.hpp"
#include "Socket.hpp"
#include "Events.hpp"
#include "Connection.hpp"


class Daemon {
private:
    Config                            *config;
    std::map<int, IEventSubscriber *> subscriber;
    std::set<Connection *>            connections;
    Events                            *events;

    static void registerSignal();
    void removeExpiredConnections();
    void processEvent(Connection *connection, int fd, size_t bytes_available, int16_t filter, bool eof);
    void processPreviousStatus(Connection *connection, short prev_status);
    void processCurrentStatus(Connection *connection);
    void subscribe(int fd, short type, Connection *connection);
    void unsubscribe(Connection *connection);
public:
    Daemon(Config *config, const std::map<int, Socket *> &sockets, Events *events);
    void run();
};

#endif //WEBSERV_DAEMON_HPP
