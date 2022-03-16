#include "AHttpMessage.hpp"

bool
AHttpMessage::parse(const std::string &src, std::size_t &token_start, const std::string &token_delim, bool delim_exact,
                    std::size_t limit, std::string &buff) {
    token_start = src.find_first_not_of(" \t\r\n", token_start);

    if (token_start == std::string::npos) {
        return false;
    }

    std::size_t line_end = src.find_first_of("\r\n", token_start);

    if (line_end == std::string::npos) {
        line_end = src.length();
    }

    std::size_t token_end = src.find_first_of(token_delim, token_start);

    if (token_end == std::string::npos && delim_exact) {
        return false;
    }

    if (token_end == std::string::npos) {
        token_end = line_end;
    }

    buff = src.substr(token_start, token_end - token_start);

    if (buff.empty() || buff.length() > limit) {
        return false;
    }

    token_start = token_end;
    return true;
}

std::map<std::string, std::string> &AHttpMessage::getHeaders() {
    return this->headers;
}

std::string &AHttpMessage::getBody() {
    return this->body;
}

void AHttpMessage::setHeader(const std::string key, const std::string value) {
    this->headers[key] = value;
}
