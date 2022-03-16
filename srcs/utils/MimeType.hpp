#ifndef WEBSERV_MIMETYPE_HPP
#define WEBSERV_MIMETYPE_HPP

#include <iostream>
#include <map>
#include <list>
#include <fstream>

class MimeType {
protected:
    static std::multimap<std::string, std::string> _types;
private:
    static void skipTokens(std::list<std::string>::iterator &it,
                    std::list<std::string>::iterator &end, int num);
    static void tokenizeFileStream(std::string const &file_path, std::list<std::string> &res);

public:
    MimeType(std::string const &path_to_conf);

    static const char *getType(std::string const &file_path);

    class MimeTypeException : public std::exception
    {
        const std::string m_msg;
    public:
        MimeTypeException(const std::string &msg) : m_msg(msg) {}
        ~MimeTypeException() throw() {};


        const char *what() const throw()
        {
            std::cout << "[MineType] Error: " << m_msg << "\n";
            return exception::what();
        }
    };
};


#endif
