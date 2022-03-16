#include "Config.hpp"
#include "Socket.hpp"
#include "Events.hpp"
#include "Daemon.hpp"

Socket *getSocket(std::map<int, Socket *> sockets, std::vector<Server *>::iterator servers_it) {
    std::map<int, Socket *>::iterator it;

    for (it = sockets.begin(); it != sockets.end(); ++it) {
        Server *server;

        server = it->second->getDefaultServer();
        if ((*servers_it)->getHost() == server->getHost() && (*servers_it)->getPort() == server->getPort()) {
            return it->second;
        }
    }

    return nullptr;
}

void run(const std::string &path_to_file) {
    Config                  *config;
    Events                  *events;
    std::map<int, Socket *> sockets;
    std::vector<Server *>   servers;

    config  = new Config(path_to_file);
    events  = new Events(1000);
    servers = config->getServers();
    MimeType("./config/mime-type.conf");

    if (!servers.empty()) {
        std::vector<Server *>::iterator servers_it;
        servers_it = servers.begin();

        while (servers_it != servers.end()) {
            Socket *old_socket;
            Socket *socket;
            int    socket_fd;

            old_socket = getSocket(sockets, servers_it);
            if (old_socket != nullptr) {
                old_socket->appendServer(*servers_it);
                servers_it++;
                continue;
            }

            socket    = new Socket((*servers_it));
            socket_fd = socket->getFd();

            sockets.insert(std::map<int, Socket *>::value_type(socket_fd, socket));
            events->subscribe(socket_fd, EVFILT_READ);
            servers_it++;
        }

        Daemon daemon(config, sockets, events);
        daemon.run();
    }
}

int main(int argc, char **argv) {
    std::string path_to_file = "./config/default.conf";

    if (argc > 1) {
        path_to_file = argv[1];
    }

    try {
        run(path_to_file);
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
}
