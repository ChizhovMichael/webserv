#include "Server.hpp"

// default
Server::Server():
    _host(inet_addr("127.0.0.1")),
    _port(80),
    _server_name(""),
    _mime_conf_path(""),
    _error_pages(),
    _locations(),
    _client_max_body_size(1000000000)
{}

Server::~Server()
{
    std::vector<Location *>::iterator l_it = _locations.begin();

    while (l_it != _locations.end())
    {
        delete *l_it;
        ++l_it;
    }
    _locations.clear();
}

in_addr_t Server::getHost() const
{
    return _host;
}

uint16_t Server::getPort() const
{
    return _port;
}

std::string Server::getServerName() const
{
    return _server_name;
}

unsigned long Server::getClientMaxBodySize() const
{
    return _client_max_body_size;
}

std::string Server::getMimeConfPath() const
{
    return _mime_conf_path;
}

std::map<short, std::string> Server::getErrorPages() const
{
    return _error_pages;
}

std::vector<Location *> Server::getLocations() const
{
    return _locations;
}

void Server::setHost(in_addr_t host)
{
    _host = host;
}

void Server::setPort(uint16_t port)
{
    _port = port;
}

void Server::setServerName(std::string const &server_name)
{
    _server_name = server_name;
}

void Server::setClientMaxBodySize(unsigned long client_max_body_size)
{
    _client_max_body_size = client_max_body_size;
}

void Server::setMimeConfPath(std::string const &mime_conf_path)
{
    _mime_conf_path = mime_conf_path;
}

void Server::setErrorPages(std::map<short, std::string> error_pages)
{
    _error_pages = error_pages;
}

void Server::setErrorPage(std::pair<short, std::string> error_page)
{
    _error_pages.insert(error_page);
}

void Server::setLocations(std::vector<Location *> locations)
{
    _locations = locations;
}

void Server::setLocation(Location * location)
{
    _locations.push_back(location);
}

const Location *Server::getLocationFromRequestUri(const std::string &uri) {
    std::vector<Location *>::const_iterator it;
    std::string                             path;
    size_t                                  dot_pos;
    std::string                             uri_ext;
    std::string                             loc_ext;

    if (uri.empty()) {
        return nullptr;
    }

    dot_pos = uri.find_last_of('.');

    if (dot_pos != std::string::npos) {
        uri_ext = uri.substr(dot_pos);
    }

    for (it = _locations.begin(); it != _locations.end(); ++it) {
        path = (*it)->getPath();

        if (path == uri || path == (uri + '/')) {
            break;
        }

        if (uri.find(path) == 0) {
            break;
        }

        if (!(*it)->getCgiPass().empty() && !uri_ext.empty()) {
            dot_pos = path.find_last_of('.');
            if (dot_pos != std::string::npos) {
                loc_ext = path.substr(dot_pos);
            }
            if (!loc_ext.empty() && loc_ext == uri_ext) {
                break;
            }
            loc_ext = "";
        }
    }

    if (it == _locations.end()) {
        return nullptr;
    }

    return *it;
}

std::string Server::getCustomErrorPagePath(short code) const {
    std::map<short, std::string>::const_iterator it;

    it = _error_pages.find(code);
    if (it == _error_pages.end()) {
        return "";
    } else {
        return it->second;
    }
}
