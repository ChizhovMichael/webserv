#ifndef WEBSERV_PRINT_HPP
#define WEBSERV_PRINT_HPP

#include <iostream>
#include "HttpRequest.hpp"
#include "Connection.hpp"

#define PRINT_LEVEL 1

class Connection;

class Print {

};

enum Code {
    FG_DEFAULT       = 39,
    FG_LIGHT_GREEN   = 92,
    FG_LIGHT_YELLOW  = 93,
    FG_LIGHT_CYAN    = 96
};

std::ostream &operator<<(std::ostream &os, Code cod);

std::ostream &operator<<(std::ostream &out,  HttpRequest &c);

std::ostream &operator<<(std::ostream &out, HttpResponse &c);

std::ostream &operator<<(std::ostream &out, Connection &c);

#endif //WEBSERV_PRINT_HPP
