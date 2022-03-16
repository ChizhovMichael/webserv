#ifndef WEBSERV_HTTPRESPONSE_HPP
#define WEBSERV_HTTPRESPONSE_HPP

#include<fstream>
#include <unistd.h>
#include "AHttpMessage.hpp"
#include "HttpRequest.hpp"
#include "Server.hpp"
#include "Cgi.hpp"
#include "./utils/Path.hpp"
#include "./utils/MimeType.hpp"

#define CGI_BUF_SIZE 1048576

class HttpRequest;

class Cgi;

class HttpResponse : public AHttpMessage {
private:
    std::string       protocol;
    u_int16_t         status_code;
    std::string       status_reason;
    std::string       response_string;
    std::size_t       body_size;
    std::vector<char> _headers_vec;
    size_t            pos;
    const Location    *location;
    Server            *server;
    HttpRequest       *request;
    Cgi               *cgi;

    HTTPStatus writeFileToBuffer(const std::string &file_path);
    void setError(HTTPStatus code);
    void prepareData();
public:
    static const std::string HTTP_REASON_CONTINUE;
    static const std::string HTTP_REASON_SWITCHING_PROTOCOLS;
    static const std::string HTTP_REASON_PROCESSING;
    static const std::string HTTP_REASON_OK;
    static const std::string HTTP_REASON_CREATED;
    static const std::string HTTP_REASON_ACCEPTED;
    static const std::string HTTP_REASON_NONAUTHORITATIVE;
    static const std::string HTTP_REASON_NO_CONTENT;
    static const std::string HTTP_REASON_RESET_CONTENT;
    static const std::string HTTP_REASON_PARTIAL_CONTENT;
    static const std::string HTTP_REASON_MULTI_STATUS;
    static const std::string HTTP_REASON_ALREADY_REPORTED;
    static const std::string HTTP_REASON_IM_USED;
    static const std::string HTTP_REASON_MULTIPLE_CHOICES;
    static const std::string HTTP_REASON_MOVED_PERMANENTLY;
    static const std::string HTTP_REASON_FOUND;
    static const std::string HTTP_REASON_SEE_OTHER;
    static const std::string HTTP_REASON_NOT_MODIFIED;
    static const std::string HTTP_REASON_USE_PROXY;
    static const std::string HTTP_REASON_TEMPORARY_REDIRECT;
    static const std::string HTTP_REASON_PERMANENT_REDIRECT;
    static const std::string HTTP_REASON_BAD_REQUEST;
    static const std::string HTTP_REASON_UNAUTHORIZED;
    static const std::string HTTP_REASON_PAYMENT_REQUIRED;
    static const std::string HTTP_REASON_FORBIDDEN;
    static const std::string HTTP_REASON_NOT_FOUND;
    static const std::string HTTP_REASON_METHOD_NOT_ALLOWED;
    static const std::string HTTP_REASON_NOT_ACCEPTABLE;
    static const std::string HTTP_REASON_PROXY_AUTHENTICATION_REQUIRED;
    static const std::string HTTP_REASON_REQUEST_TIMEOUT;
    static const std::string HTTP_REASON_CONFLICT;
    static const std::string HTTP_REASON_GONE;
    static const std::string HTTP_REASON_LENGTH_REQUIRED;
    static const std::string HTTP_REASON_PRECONDITION_FAILED;
    static const std::string HTTP_REASON_REQUEST_ENTITY_TOO_LARGE;
    static const std::string HTTP_REASON_REQUEST_URI_TOO_LONG;
    static const std::string HTTP_REASON_UNSUPPORTED_MEDIA_TYPE;
    static const std::string HTTP_REASON_REQUESTED_RANGE_NOT_SATISFIABLE;
    static const std::string HTTP_REASON_EXPECTATION_FAILED;
    static const std::string HTTP_REASON_IM_A_TEAPOT;
    static const std::string HTTP_REASON_ENCHANCE_YOUR_CALM;
    static const std::string HTTP_REASON_MISDIRECTED_REQUEST;
    static const std::string HTTP_REASON_UNPROCESSABLE_ENTITY;
    static const std::string HTTP_REASON_LOCKED;
    static const std::string HTTP_REASON_FAILED_DEPENDENCY;
    static const std::string HTTP_REASON_UPGRADE_REQUIRED;
    static const std::string HTTP_REASON_PRECONDITION_REQUIRED;
    static const std::string HTTP_REASON_TOO_MANY_REQUESTS;
    static const std::string HTTP_REASON_REQUEST_HEADER_FIELDS_TOO_LARGE;
    static const std::string HTTP_REASON_UNAVAILABLE_FOR_LEGAL_REASONS;
    static const std::string HTTP_REASON_INTERNAL_SERVER_ERROR;
    static const std::string HTTP_REASON_NOT_IMPLEMENTED;
    static const std::string HTTP_REASON_BAD_GATEWAY;
    static const std::string HTTP_REASON_SERVICE_UNAVAILABLE;
    static const std::string HTTP_REASON_GATEWAY_TIMEOUT;
    static const std::string HTTP_REASON_VERSION_NOT_SUPPORTED;
    static const std::string HTTP_REASON_VARIANT_ALSO_NEGOTIATES;
    static const std::string HTTP_REASON_INSUFFICIENT_STORAGE;
    static const std::string HTTP_REASON_LOOP_DETECTED;
    static const std::string HTTP_REASON_NOT_EXTENDED;
    static const std::string HTTP_REASON_NETWORK_AUTHENTICATION_REQUIRED;
    static const std::string HTTP_REASON_UNKNOWN;
//    static const std::string DATE;
//    static const std::string SET_COOKIE;

    HttpResponse(Server *server, HttpRequest *request);
    ~HttpResponse();
    uint16_t getStatusCode() const;
    static const std::string &getReasonForStatus(HTTPStatus status);
    void setResponseString(HTTPStatus status);
    const std::string &getResponseString() const;

    bool isCgi();
    void processCgiRequest(const std::string &ip);
    void processGetRequest();
    void processPostRequest();
    void processDeleteRequest();
    void processPutRequest();

    Cgi *getCgi() const;
    void setCgi(Cgi *_cgi);

    int send(int fd, size_t bytes);
    void writeToCgi(HttpRequest *req, size_t bytes);
    bool readCgi(size_t bytes, bool eof);
    bool parseCgiHeaders(size_t end);
    bool checkCgiHeaders(size_t &i, const std::string &sep, size_t max, std::string &token);
};

#endif //WEBSERV_HTTPRESPONSE_HPP
