#include "Print.hpp"

std::ostream &operator<<(std::ostream &os, Code cod) {
    os << "\033[" << (int) cod << "m";
    return os;
}

std::ostream &operator<<(std::ostream &out, HttpRequest &c) {
    if (PRINT_LEVEL > 0) {
        out << FG_LIGHT_YELLOW << ">>>> [REQUEST] <<<<" << std::endl;
        out << "uri - " << c.getRequestUri() << std::endl;
        out << "code - " << c.getParsingError() << std::endl;
        out << "Content-Length - " << c.getContentLength() << std::endl << std::endl;
    }
    if (PRINT_LEVEL > 1) {
        const std::map<std::string, std::string>           &res = c.getHeaders();
        std::map<std::string, std::string>::const_iterator it;
        for (it = res.begin(); it != res.end(); ++it) {
            out << it->first << ": " << it->second << std::endl;
        }
        out << std::endl;
    }
    if (PRINT_LEVEL > 2) {
        out << c.getBody() << std::endl;
    }
    if (PRINT_LEVEL > 0) {
        out << std::endl << FG_DEFAULT;
    }
    return out;
}

std::ostream &operator<<(std::ostream &out, HttpResponse &c) {
    if (PRINT_LEVEL > 0) {
        out << FG_LIGHT_GREEN << ">>>> [RESPONSE] <<<<" << std::endl;
        out << "response string: " << c.getResponseString() << std::endl;
        out << "status code: [" << c.getStatusCode() << "] "
            << HttpResponse::getReasonForStatus(static_cast<HttpResponse::HTTPStatus>(c.getStatusCode()))
            << std::endl;
        out << "Body size is: " << c.getBody().size() << std::endl << std::endl;
    }
    if (PRINT_LEVEL > 1) {
        const std::map<std::string, std::string>           &res = c.getHeaders();
        std::map<std::string, std::string>::const_iterator it;
        for (it = res.begin(); it != res.end(); ++it) {
            out << it->first << ": " << it->second << std::endl;
        }
        out << std::endl;
    }
    if (PRINT_LEVEL > 2) {
        out << c.getBody() << std::endl;
    }
    if (PRINT_LEVEL > 0) {
        out << std::endl << FG_DEFAULT;
    }
    return out;
}

std::ostream &operator<<(std::ostream &out, Connection &c) {
    if (PRINT_LEVEL > 0) {
        out << FG_LIGHT_CYAN << ">>>> [Connection] <<<<" << std::endl;

        Server *server = c.getServer();
        if (server) {
            out << "Host: " << std::to_string(server->getHost()) << " "
                << "Port: " << std::to_string(server->getPort()) << std::endl;
        }

        time_t t = c.getConnectionTimeout();
        char   buf[100];
        std::strftime(buf, 100, "%a, %d %b %Y %H:%M:%S %Z", gmtime(&t));
        out << "Last updated: " << buf << std::endl;
    }
    if (PRINT_LEVEL > 1) {
        out << "fd " << c.getConnectionFd() << std::endl;
    }
    if (PRINT_LEVEL > 0) {
        out << FG_DEFAULT << std::endl;
    }
    return out;
}
