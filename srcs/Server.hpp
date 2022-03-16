#ifndef WEBSERV_SERVER_HPP
#define WEBSERV_SERVER_HPP

# include <iostream>
# include <arpa/inet.h>
# include <vector>
# include <map>
# include "Location.hpp"
# include "IDirective.hpp"

class Server: public IDirective
{
private:
    in_addr_t                           _host;
    uint16_t                            _port;
    std::string                         _server_name;
    std::string                         _mime_conf_path;
    std::map<short, std::string>        _error_pages;
    std::vector<Location *>             _locations;
    unsigned long                       _client_max_body_size;

    Server(const Server &a);
    Server &operator=(const Server &a);
public:
    Server();
    ~Server();

    in_addr_t getHost() const;
    uint16_t getPort() const;
    std::string getServerName() const;
    unsigned long getClientMaxBodySize() const;
    std::string getMimeConfPath() const;
    std::map<short, std::string> getErrorPages() const;
    std::vector<Location *> getLocations() const;

    void setHost(in_addr_t host);
    void setPort(uint16_t host);
    void setServerName(std::string const &server_name);
    void setClientMaxBodySize(unsigned long client_max_body_size);
    void setMimeConfPath(std::string const &mime_conf_path);
    void setErrorPages(std::map<short, std::string> error_pages);
    void setErrorPage(std::pair<short, std::string> error_page);
    void setLocations(std::vector<Location *> locations);
    void setLocation(Location *location);

    const Location *getLocationFromRequestUri(const std::string &basicString);
    std::string getCustomErrorPagePath(short code) const;
};


#endif //WEBSERV_SERVER_HPP
