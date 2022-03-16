#ifndef WEBSERV_FILESTATS_HPP
#define WEBSERV_FILESTATS_HPP

#include <string>
#include <sys/stat.h>
#include <ctime>

class FileStats {
    struct stat file_stats;
public:
    FileStats(const std::string &path_toFile);
    std::string getTimeModified();
    std::string getSizeInMb() const;
    bool isDir() const;
};

#endif //WEBSERV_FILESTATS_HPP
