#ifndef WEBSERV_CGI_HPP
#define WEBSERV_CGI_HPP

# include <string>
# include <map>
# include <csignal>
# include "HttpRequest.hpp"
# include "Location.hpp"

class HttpRequest;

class Cgi {
private:
    std::string                        _path;
    std::map<std::string, std::string> _env;
    int                                _reqFd;
    int                                _resFd;
    pid_t                              _pid;
    size_t                             _pos;
    bool                               _headers_parsed;
    int                                _exit_status;
public:
    explicit Cgi(const std::string &path);
    ~Cgi();

    const std::string &getPath() const;

    const std::map<std::string, std::string> &getEnv() const;

    char **getEnvAsArray(void);

    bool prepareCgiEnv(HttpRequest *request, const std::string &absolute_path, const std::string &client_ip,
                       const std::string &serv_port, const std::string &cgi_exec);

    pid_t getPid() const;

    void setPid(pid_t pid);

    int getReqFd() const;

    void setReqFd(int reqFd);

    int getResFd() const;

    void setResFd(int resFd);

    size_t getPos() const;

    void setPos(size_t pos);

    bool isHeadersParsed() const;

    void setHeadersParsed(bool);
};

#endif //WEBSERV_CGI_HPP
