#ifndef WEBSERV_HTTPREQUEST_HPP
#define WEBSERV_HTTPREQUEST_HPP

#include "AHttpMessage.hpp"
#include "HttpResponse.hpp"

class HttpResponse;

class HttpRequest : public AHttpMessage {
private:
    std::string   method;
    std::string   request_uri;
    std::string   query_string;
    std::string   uri_no_query;
    std::string   absolute_path;
    std::string   http_v;
    bool          chunked;
    unsigned long content_length;
    bool          ready;
    uint16_t      parsing_error;

public:
    enum Limits {
        MAX_METHOD            = 8,
        MAX_URI               = 2000,
        MAX_V                 = 100,
        MAX_HEADERS           = 100,
        MAX_NAME              = 100,
        MAX_VALUE             = 1000,
        MAX_MESSAGE           = 10000,
        MAX_DEFAULT_BODY_SIZE = 1000000
    };

    HttpRequest();

    void setReady(bool ready);
    bool getReady() const;
    const std::string &getRequestUri() const;
    void setParsingError(uint16_t status);
    uint16_t getParsingError() const;
    unsigned long getContentLength() const;
    void setContentLength(unsigned long len);
    bool getChunked() const;
    const std::string &getUriNoQuery() const;
    const std::string &getMethod() const;
    const std::string &getAbsolutPath() const;
    void setAbsolutPath(const std::string &path);

    bool headersSent(const std::string &req);
    bool parseRequestLine(const std::string &req, size_t &pos);
    bool parseHeaders(const std::string &buffe, size_t &pos);
    bool processUri();
    bool processHeaders();
    bool checkContentLength(unsigned long size);

    const std::string &getQueryString() const;
};

#endif //WEBSERV_HTTPREQUEST_HPP
