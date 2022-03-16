#include "Socket.hpp"


Socket::Socket(Server *server) {
    int         socket_fd;
    int         opt  = 1;
    sockaddr_in addr = {};

    this->appendServer(server);

    socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_fd == -1) {
        throw SocketException(std::strerror(errno));
    }

    this->fd = socket_fd;

    memset(addr.sin_zero, 0, 8);
    addr.sin_len         = sizeof(addr);
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(server->getPort());
    addr.sin_addr.s_addr = server->getHost();

    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) == -1
        || fcntl(socket_fd, F_SETFL, O_NONBLOCK) == -1
        || bind(socket_fd, (struct sockaddr *) &addr, sizeof(addr)) == -1
        || listen(socket_fd, MAX_AWAIT_CONN) == -1) {
        close(socket_fd);
        throw SocketException(std::strerror(errno));
    }
}

int Socket::getFd() const {
    return this->fd;
}

Server *Socket::getDefaultServer() const {
    return this->servers[0];
}

std::vector<Server *> Socket::getServers() const {
    return this->servers;
}

void Socket::appendServer(Server *server) {
    this->servers.push_back(server);
}

Socket::SocketException::SocketException(const char *msg) : m_msg(msg) {}

Socket::SocketException::~SocketException() throw() {}

const char *Socket::SocketException::what() const throw() {
    std::cerr << "SocketError: ";
    return this->m_msg;
}
