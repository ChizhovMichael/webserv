#ifndef WEBSERV_PATH_HPP
#define WEBSERV_PATH_HPP

#include <list>
#include <string>
#include <sys/stat.h>
#include <dirent.h>
#include "FileStats.hpp"

class Path {
public:
    static const std::string AUTOINDEX_HTML;
    static bool isDirectory(const std::string &path);
    static std::string getAutoIndexHtml(const std::string &path, const std::string &uri_path);
    static bool isNotEmptyDirectory(const std::string &path);
    static bool fileExistsAndWritable(const std::string &name);
    static bool folderExistsAndWritable(const std::string &name);
    static std::string getFileNameFromPath(const std::string &path);
    static bool checkIfPathExists(const std::string &path);
    static std::string getFullPath(const std::string &loc_path, const std::string &root, const std::string &uri);
    static void removeLocFromUri(const std::string &location, std::string &uri);
    static std::list<std::string> toSlashList(const std::string &s);
    static std::string urlDecode(std::string &src);
};

#endif //WEBSERV_PATH_HPP
