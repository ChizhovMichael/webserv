#include "FileStats.hpp"

FileStats::FileStats(const std::string &path_to_file) : file_stats() {
    stat(path_to_file.data(), &this->file_stats);
}

std::string FileStats::getTimeModified() {
    struct tm   *clock;
    std::string human_time;

    clock = gmtime(&(this->file_stats.st_mtimespec.tv_sec));
    return asctime(clock);
}

std::string FileStats::getSizeInMb() const {
    return std::to_string((float) this->file_stats.st_size / 1000000);
}

bool FileStats::isDir() const {
    return S_ISDIR(this->file_stats.st_mode);
}
