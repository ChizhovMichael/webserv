#include "Cgi.hpp"

Cgi::Cgi(const std::string &path) {
    _path           = path;
    _pid            = -1;
    _reqFd          = -1;
    _resFd          = -1;
    _pos            = 0;
    _headers_parsed = false;
    _exit_status    = 0;
}

Cgi::~Cgi() {
    if (waitpid(_pid, &_exit_status, WNOHANG) == 0) {
        kill(_pid, SIGKILL);
    }
}

const std::string &Cgi::getPath() const {
    return _path;
}

const std::map<std::string, std::string> &Cgi::getEnv() const {
    return _env;
}

bool Cgi::isHeadersParsed() const {
    return _headers_parsed;
}

void Cgi::setHeadersParsed(bool status) {
    _headers_parsed = status;
}

bool Cgi::prepareCgiEnv(HttpRequest *request, const std::string &absolute_path, const std::string &client_ip,
                        const std::string &serv_port, const std::string &cgi_exec) {
    std::map<std::string, std::string>::iterator it;

    _env["REDIRECT_STATUS"] = "CGI";
    _env["AUTH_TYPE"]       = "";
    _env["CONTENT_LENGTH"]  = std::to_string(request->getContentLength());

    it                        = request->getHeaders().find("Content-Type");
    if (it != request->getHeaders().end()) {
        _env["CONTENT_TYPE"] = it->second;
    } else {
        _env["CONTENT_TYPE"] = "";
    }
    _env["GATEWAY_INTERFACE"] = std::string("CGI/1.1");
    if (!cgi_exec.empty()) {
        _env["SCRIPT_NAME"] = cgi_exec;
    } else {
        _env["SCRIPT_NAME"] = "";
    }
    _env["SCRIPT_FILENAME"]   = absolute_path;
    _env["PATH_TRANSLATED"]   = request->getRequestUri();
    _env["PATH_INFO"]         = request->getRequestUri();
    _env["QUERY_STRING"]      = request->getQueryString();
    _env["REQUEST_URI"]       = request->getRequestUri();
    _env["REMOTE_ADDR"]       = client_ip;
    _env["REMOTE_HOST"]       = client_ip;
    _env["REMOTE_IDENT"]      = "";
    _env["REMOTE_USER"]       = "";
    _env["REQUEST_METHOD"]    = request->getMethod();

    it                      = request->getHeaders().find("Host");
    if (it == request->getHeaders().end()) {
        _env["SERVER_NAME"] = "0";
    } else {
        _env["SERVER_NAME"] = it->second;
    }
    _env["SERVER_PORT"]     = serv_port;
    _env["SERVER_PROTOCOL"] = "HTTP/1.1";
    _env["SERVER_SOFTWARE"] = "Weebserv/1.0";

    std::map<std::string, std::string>           tmp_map;
    std::map<std::string, std::string>::iterator tmp_map_iter;

    for (it = request->getHeaders().begin(); it != request->getHeaders().end();) {
        std::string tmp = it->first;
        std::replace(tmp.begin(), tmp.end(), '-', '_');
        std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
        tmp_map["HTTP_" + tmp] = it->second;
        tmp_map_iter = it;
        ++tmp_map_iter;
        request->getHeaders().erase(it);
        it = tmp_map_iter;
    }
    _env.insert(tmp_map.begin(), tmp_map.end());

    return true;
}

char **Cgi::getEnvAsArray(void) {
    try {
        char **res = new char *[_env.size() + 1];
        int  i     = 0;

        for (std::map<std::string, std::string>::const_iterator it = _env.begin(); it != _env.end(); ++it) {
            if (it->second.empty()) {
                continue;
            }
            std::string tmp = it->first + "=" + it->second;
            res[i] = new char[tmp.size() + 1];
            strcpy(res[i], tmp.data());
            ++i;
        }

        res[i] = nullptr;

        return (res);
    }
    catch (std::exception &e) {
        return nullptr;
    }
}

int Cgi::getReqFd() const {
    return _reqFd;
}

void Cgi::setReqFd(int reqFd) {
    _reqFd = reqFd;
}

int Cgi::getResFd() const {
    return _resFd;
}

void Cgi::setResFd(int resFd) {
    _resFd = resFd;
}

pid_t Cgi::getPid() const {
    return _pid;
}

void Cgi::setPid(pid_t pid) {
    _pid = pid;
}

size_t Cgi::getPos() const {
    return _pos;
}

void Cgi::setPos(size_t pos) {
    _pos = pos;
}
