#ifndef WEBSERV_CONFIG_HPP
# define WEBSERV_CONFIG_HPP

# include <vector>
# include <stdint.h>
# include <netinet/in.h>
# include <unistd.h>
# include <fstream>
# include <iterator>
# include <sstream>
# include <algorithm>
# include <arpa/inet.h>
# include "Server.hpp"
# include "IDirective.hpp"

struct config_base
{
public:
    std::vector<std::string> _methods;
    std::vector<std::string> _directs;

    config_base()
    : _methods(), _directs()
    {
        _methods.push_back("GET");
        _methods.push_back("POST");
        _methods.push_back("DELETE");
        _methods.push_back("PUT");

        _directs.push_back("listen");
        _directs.push_back("port");
        _directs.push_back("server_name");
        _directs.push_back("client_max_body_size");
        _directs.push_back("mime_conf_path");
        _directs.push_back("error_page");
        _directs.push_back("location");
        _directs.push_back("return");
        _directs.push_back("root");
        _directs.push_back("methods");
        _directs.push_back("file_upload");
        _directs.push_back("upload_tmp_path");
        _directs.push_back("index");
        _directs.push_back("autoindex");
        _directs.push_back("cgi_pass");
    }
};

template <typename T>
int _getMethodIndex(std::vector<T> methods, T const &method)
{
    typename std::vector<T>::iterator itr;

    itr = std::find(methods.begin(), methods.end(), method);
    if (itr == methods.end())
    {
        throw "method not supported";
    }
    return std::distance(methods.begin(), itr);
}

template <typename T>
int _getDirectIndex(std::vector<T> directs, T const &direct)
{
    typename std::vector<T>::iterator itr;

    itr = std::find(directs.begin(), directs.end(), direct);
    if (itr == directs.end())
    {
        throw "directive not supported";
    }
    return std::distance(directs.begin(), itr);
}

class Config: public config_base
{
private:
    typedef config_base         base;

    std::string                 _path_to_file;
    std::vector<std::string>    _config;
    std::vector<Server *>       _servers;
    in_addr_t                   _default_host;
    uint16_t                    _default_port;

    Config &operator=(const Config &other);
    Config(const Config &other);

    void parse(const std::string &path_to_file);

public:
    typedef std::vector<std::string>::iterator  iterator;
    using                                       base::_methods;
    using                                       base::_directs;

    Config(const std::string &path_to_file);
    ~Config();

    const std::vector<Server *> &getServers() const;
    in_addr_t getDefaultHost() const;
    uint16_t getDefaultPort() const;

    void setDefault(Server *server);

    class ConfigException : public std::exception
    {
    private:
        std::string msg;
    public:
        ConfigException();
        ConfigException(const std::string &msg);
        ~ConfigException() throw();
        const char *what() const throw();
    };

    bool isComment(std::string const &str) const;

    bool isDirectiveLine(std::string const &str) const;

    void parseServerData(iterator value);

    void checkTrashDirective(std::string const &name) const;

    void parseListen(IDirective *server, iterator &value);

    void parsePort(IDirective *server, iterator &value);

    void parseServerName(IDirective *server, iterator &value);

    void parseClientMaxBodySize(IDirective *server, iterator &value);

    void parseMimeConfPath(IDirective *server, iterator &value);

    void parseErrorPages(IDirective *server, iterator &value);

    void parseLocation(IDirective *server, iterator &value);

    void parseRedirection(IDirective *server, iterator &value);

    void parseRoot(IDirective *server, iterator &value);

    void parseMethods(IDirective *server, iterator &value);

    void parseFileUpload(IDirective *server, iterator &value);

    void parseUploadTmpPath(IDirective *server, iterator &value);

    void parseIndex(IDirective *server, iterator &value);

    void parseAutoindex(IDirective *server, iterator &value);

    void parseCgiPass(IDirective *server, iterator &value);
};

template <typename ForwardIt>
ForwardIt myNext(
    ForwardIt it, 
    typename std::iterator_traits<ForwardIt>::difference_type n = 1
)
{
    std::advance(it, n);
    return it;
}

template <typename ForwardIt>
ForwardIt myPrev(
    ForwardIt it, 
    typename std::iterator_traits<ForwardIt>::difference_type n = -1
)
{
    std::advance(it, n);
    return it;
}


#endif //WEBSERV_CONFIG_HPP
