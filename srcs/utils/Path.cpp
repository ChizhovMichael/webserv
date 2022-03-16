#include "Path.hpp"

bool Path::isDirectory(const std::string &path) {
    struct stat s = {};

    if (stat(path.data(), &s) == 0) {
        return S_ISDIR(s.st_mode);
    }
    return false;
}

std::string Path::getAutoIndexHtml(const std::string &path, const std::string &uri_path) {
    DIR           *dp;
    struct dirent *di_struct;
    int           i = 0;
    std::string   table;

    dp = opendir(path.data());
    table += "<h1>" + uri_path + "</h1>";
    table += "<table>";
    table += "<tr> <th>File name</th> <th>File size</th> <th>Last modified</th> </tr>";
    if (dp != nullptr) {
        while ((di_struct = readdir(dp)) != nullptr) {
            FileStats file(path + "/" + di_struct->d_name);
            table += "<tr>";

            table += "<td><a href=\"" + uri_path;
            table += di_struct->d_name;
            if (file.isDir()) {
                table += "/";
            }
            table += "\">" + std::string(di_struct->d_name) + "</a></td>";
            table += "<td>" + file.getSizeInMb() + "</td>";
            table += "<td>" + file.getTimeModified() + "</td>";
            table += "</tr>";
            i++;
        }
        closedir(dp);
    }
    table += "</table>";

    return table;
}

const std::string Path::AUTOINDEX_HTML = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><title></title></head><style>table {border: 1px solid #ccc;background-color: #f8f8f8;border-collapse: collapse;margin: 0;padding: 0;width: 100%;table-layout: fixed;text-align: left;}table td:last-child {border-bottom: 0;}</style><body></body></html>";

bool Path::isNotEmptyDirectory(const std::string &path) {
    int           n    = 0;
    struct dirent *d;
    DIR           *dir = opendir(path.data());

    if (dir == nullptr) {
        return false;
    }
    while ((d = readdir(dir)) != nullptr) {
        if (++n > 2) {
            break;
        }
    }
    closedir(dir);
    if (n > 2) {
        return true;
    } else {
        return false;
    }
}

bool Path::fileExistsAndWritable(const std::string &name) {
    FILE *file;

    if ((file = std::fopen(name.c_str(), "r+")) != nullptr) {
        fclose(file);
        return true;
    } else {
        return false;
    }
}

bool Path::folderExistsAndWritable(const std::string &name) {
    struct stat st = {};

    if (stat(name.data(), &st) < 0) {
        return false;
    }
    if (S_ISDIR(st.st_mode) && (st.st_mode & S_IWUSR)) {
        return true;
    }
    return false;
}

std::string Path::getFileNameFromPath(const std::string &path) {
    size_t pos = path.find_last_of('/');

    if (pos == std::string::npos) {
        return "";
    } else {
        return path.substr(pos + 1);
    }
}

bool Path::checkIfPathExists(const std::string &path) {
    size_t      pos;
    std::string new_path;

    pos = path.find_last_of('/');
    if (pos == std::string::npos) {
        return false;
    }
    new_path = path.substr(0, pos);
    if (!new_path.empty() && isDirectory(new_path)) {
        return true;
    }
    return false;
}

std::string Path::getFullPath(const std::string &loc_path, const std::string &root, const std::string &uri) {
    std::string res = uri;

    if (*loc_path.begin() == '*') {
        res.replace(0, 1, root);
        return res;
    }
    res.replace(0, 1, root);
    return res;
}

void Path::removeLocFromUri(const std::string &location, std::string &uri) {
    std::list<std::string>::iterator it;
    std::list<std::string>::iterator loc_it;
    std::list<std::string>::iterator tmp;
    int                              delete_count = 0;
    std::list<std::string>           list_a;
    std::list<std::string>           loc_tok;

    list_a      = Path::toSlashList(uri);
    loc_tok     = Path::toSlashList(location);
    it          = list_a.begin();
    for (loc_it = loc_tok.begin(); loc_it != loc_tok.end(); ++loc_it) {
        if (it != list_a.end()) {
            if (*it != "/" && *it == *loc_it) {
                ++delete_count;
            }
            ++it;
        } else {
            break;
        }
    }
    for (it     = list_a.begin(); it != list_a.end() && delete_count > 0;) {
        tmp = it;
        ++tmp;
        if (*it != "/") {
            --delete_count;
        }
        list_a.erase(it);
        it = tmp;
    }
    uri.clear();
    for (it = list_a.begin(); it != list_a.end(); ++it) {
        uri += *it;
    }
    if (uri.empty()) {
        uri = "/";
    }
}

std::list<std::string> Path::toSlashList(const std::string &s) {
    std::list<std::string> slashList;
    std::string            res;

    for (std::string::const_iterator it = s.begin(); it != s.end(); ++it) {
        if (*it == '/') {
            if (!res.empty()) {
                slashList.push_back(res);
            }
            res.clear();
            res.push_back(*it);
            slashList.push_back(res);
            res.clear();
            continue;
        }
        res.push_back(*it);
    }
    if (*s.rbegin() != '/') {
        slashList.push_back(res);
    }
    return slashList;
}

std::string Path::urlDecode(std::string &src) {
    std::string output;
    char        ch;
    unsigned    i, ii;

    for (i = 0; i < src.length(); i++) {
        if (int(src[i]) == 37) {
            sscanf(src.substr(i + 1, 2).c_str(), "%x", &ii);
            ch = static_cast<char>(ii);
            output += ch;
            i  = i + 2;
        } else {
            output += src[i];
        }
    }

    return output;
}
