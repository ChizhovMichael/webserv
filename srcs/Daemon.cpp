#include "Daemon.hpp"

Daemon::Daemon(Config *config, const std::map<int, Socket *> &sockets, Events *events) :
        config(config),
        events(events) {
    this->subscriber.insert(sockets.begin(), sockets.end());
    (void) this->config;
}

void signal_handler(int signal) {
    std::cout << "stopping on signal" << std::endl;
    exit(signal);
}

void Daemon::registerSignal() {
    signal(SIGPIPE, SIG_IGN);
    std::signal(SIGINT, signal_handler);
    std::signal(SIGQUIT, signal_handler);
}

void Daemon::run() {
    std::pair<int, struct kevent *>             updates;
    struct kevent                               event = {};
    std::map<int, IEventSubscriber *>::iterator sub_it;
    int                                         sub_fd;
    int                                         i;

    this->registerSignal();

    while (true) {
        updates = this->events->getUpdates();
        i       = 0;
        while (i < updates.first) {
            event  = updates.second[i];
            sub_fd = static_cast<int> (event.ident);
            sub_it = this->subscriber.find(sub_fd);

            if (event.flags & EV_ERROR) {
                fprintf(stderr, "EV_ERROR: %s\n", strerror(static_cast<int>(event.data)));
            }

            if (sub_it != this->subscriber.end()) {
                if (dynamic_cast<Socket *>(sub_it->second)) {
                    Connection *connection;
                    connection = new Connection(dynamic_cast<Socket *>(sub_it->second));
                    this->subscribe(connection->getConnectionFd(), EVFILT_READ, connection);
                } else if (dynamic_cast<Connection *>(sub_it->second)) {
                    Connection *connection = dynamic_cast<Connection *>(sub_it->second);
                    this->processEvent(connection, sub_fd, event.data, event.filter, (event.flags & EV_EOF));
                }
            }
            ++i;
        }
        this->removeExpiredConnections();
    }
}

void Daemon::processEvent(Connection *connection, int fd, size_t bytes_available, int16_t filter, bool eof) {
    short       prev_status   = connection->getStatus();
    int         connection_fd = connection->getConnectionFd();
    HttpRequest *request      = connection->getRequest();

    if (connection_fd == fd && eof &&
        (filter == EVFILT_WRITE ||
         (filter == EVFILT_READ && (request == nullptr || !request->getReady()))
        )) {
        this->unsubscribe(connection);
        return;
    } else if ((connection->getStatus() == Connection::AWAIT_NEW_REQ || connection->getStatus() == Connection::UNUSED)
               && filter == EVFILT_READ && fd == connection_fd && bytes_available > 0) {
        if(!connection->parseRequest(bytes_available)) {
            this->unsubscribe(connection);
            return;
        }
        connection->prepareResponse();
    } else if (connection->getStatus() == Connection::CGI_PROCESSING) {
        connection->processCgi(fd, bytes_available, filter, eof);
    } else if (connection->getStatus() == Connection::SENDING && filter == EVFILT_WRITE && fd == connection_fd) {
        if(!connection->processResponse(bytes_available, eof)) {
            this->unsubscribe(connection);
            return;
        }
    }

    if (prev_status != connection->getStatus()) {
        this->processPreviousStatus(connection, prev_status);
        this->processCurrentStatus(connection);
    }
}

void Daemon::processPreviousStatus(Connection *connection, short prev_status) {
    if (prev_status == Connection::CGI_PROCESSING) {
        if (!connection->getRequest()->getBody().empty()) {
            this->events->unsubscribe(connection->getResponse()->getCgi()->getReqFd());
        }
        this->events->unsubscribe(connection->getResponse()->getCgi()->getResFd());
        return;
    }
}

void Daemon::processCurrentStatus(Connection *connection) {
    short status;

    status        = connection->getStatus();
    if (status == Connection::AWAIT_NEW_REQ) {
        this->subscribe(connection->getConnectionFd(), EVFILT_READ, connection);
    }
    if (status == Connection::CGI_PROCESSING) {
        /**
         * Здесь что-то
         */
        if (!connection->getRequest()->getBody().empty()) {
            this->subscribe(connection->getResponse()->getCgi()->getReqFd(), EVFILT_WRITE, connection);
        }
        this->subscribe(connection->getResponse()->getCgi()->getResFd(), EVFILT_READ, connection);
        return;
    }
    if (status == Connection::SENDING) {
        this->subscribe(connection->getConnectionFd(), EVFILT_WRITE, connection);
        return;
    }
    if (status == Connection::CLOSING) {
        this->unsubscribe(connection);
    }
}

void Daemon::removeExpiredConnections() {
    if (this->connections.empty()) {
        return;
    }

    std::vector<Connection *> to_delete;
    std::set<Connection *>::iterator connection_it;

    for (connection_it = this->connections.begin(); connection_it != this->connections.end();) {
        if ((*connection_it)->isShouldClose()) {
            to_delete.push_back(*connection_it);
        }
        ++connection_it;
    }

    std::vector<Connection *>::iterator it;
    for (it = to_delete.begin(); it != to_delete.end(); ++it) {
        this->unsubscribe(*it);
    }
}

void Daemon::subscribe(int fd, short type, Connection *connection) {
    this->subscriber.insert(std::make_pair(fd, connection));
    this->connections.insert(connection);
    this->events->subscribe(fd, type);
}

void Daemon::unsubscribe(Connection *connection) {
    std::set<Connection *>::iterator       it_c;
    std::map<int, IEventSubscriber *>::iterator it_s;

    for (it_c = this->connections.begin(); it_c != this->connections.end();) {
        if (*it_c == connection) {
            it_c = this->connections.erase(it_c);
        } else {
            ++it_c;
        }
    }

    for (it_s = this->subscriber.begin(); it_s != this->subscriber.end();) {
        if (it_s->second == connection) {
            this->events->unsubscribe(it_s->first);
            close(it_s->first);
            it_s = this->subscriber.erase(it_s);
        } else {
            ++it_s;
        }
    }

    delete connection;
}
