#include "HttpResponse.hpp"

HttpResponse::HttpResponse(Server *server, HttpRequest *request)
        : protocol("HTTP/1.1"),
          status_code(HTTP_OK),
          body_size(0),
          pos(0),
          server(server),
          request(request),
          cgi(nullptr) {
    if (this->server == nullptr) {
        this->setError(HttpResponse::HTTP_BAD_REQUEST);
    }

    std::string uri = this->request->getUriNoQuery();
    this->location = this->server->getLocationFromRequestUri(uri);
    if (this->location == nullptr) {
        this->setError(HTTP_NOT_FOUND);
        return;
    }
    Path::removeLocFromUri(this->location->getPath(), uri);
    std::string path = Path::getFullPath(this->location->getPath(), this->location->getRoot(), uri);
    if (Path::isDirectory(path) && !this->location->getAutoindex() && !this->location->getIndex().empty()) {
        if (*path.rbegin() != '/') {
            path += '/';
        }
        path += this->location->getIndex();
    }
    this->request->setAbsolutPath(path);
    if (!this->location->methodAllowed(this->request->getMethod())) {
        this->setHeader("Allow", this->location->getAllowedMethodsField());
        this->setError(HTTP_METHOD_NOT_ALLOWED);
        return;
    }
    if (!this->location->getRedirection().empty()) {
        this->setHeader("Location", this->location->getRedirection());
        this->setResponseString(HTTP_MOVED_PERMANENTLY);
        return;
    }
    if (this->server->getClientMaxBodySize() < this->request->getBody().size()) {
        this->setError(HTTP_REQUEST_ENTITY_TOO_LARGE);
        return;
    }
    if (this->request->getParsingError() != HttpRequest::HTTP_OK) {
        this->setError(static_cast<HttpRequest::HTTPStatus>(this->request->getParsingError()));
        return;
    }
    if (!this->location->getCgiPass().empty()) {
        this->cgi = new Cgi(this->location->getCgiPass());
    }
}

HttpResponse::~HttpResponse() {
    if (this->cgi != nullptr) {
        delete this->cgi;
        this->cgi = nullptr;
    }
}

uint16_t HttpResponse::getStatusCode() const {
    return this->status_code;
}

void HttpResponse::setResponseString(HTTPStatus status) {
    this->status_code   = status;
    this->status_reason = getReasonForStatus(status);
    this->response_string =
            this->protocol + " " + std::to_string(this->status_code) + " " + this->status_reason + "\r\n";
}

const std::string &HttpResponse::getResponseString() const{
    return this->response_string;
}

bool HttpResponse::isCgi() {
    return (this->location != nullptr) && !(this->location->getCgiPass().empty()) && (this->cgi != nullptr);
}

void freeEnv(char **env) {
    for (int i = 0; env[i] != nullptr; ++i) {
        free(env[i]);
    }
    free(env);
}

/**
 * CGI exec
 * @param ip
 */
void HttpResponse::processCgiRequest(const std::string &ip) {
    this->cgi->prepareCgiEnv(this->request, this->request->getAbsolutPath(), ip, std::to_string(server->getPort()),
                             this->location->getCgiPass());
    char  *argv[3];
    int   in_pipe[2];
    int   out_pipe[2];
    pid_t child_pid;
    int   res;
    char  **env = cgi->getEnvAsArray();

    if (env == nullptr) {
        return;
    }

    argv[0] = const_cast<char *>(cgi->getPath().data());
    argv[1] = const_cast<char *>(request->getAbsolutPath().data());
    argv[2] = nullptr;

    if (pipe(in_pipe) < 0) {
        freeEnv(env);
        std::cerr << "allocating pipe for child input redirect failed" << std::endl;
        return;
    }
    if (pipe(out_pipe) < 0) {
        freeEnv(env);
        close(in_pipe[0]);
        close(in_pipe[1]);
        std::cerr << "allocating pipe for child output redirect failed" << std::endl;
        return;
    }
    child_pid = fork();
    if (child_pid == 0) {
        if (dup2(in_pipe[0], STDIN_FILENO) == -1 ||
            dup2(out_pipe[1], STDOUT_FILENO) == -1 ||
            dup2(out_pipe[1], STDERR_FILENO) == -1) {
            exit(EXIT_FAILURE);
        }
        close(in_pipe[0]);
        close(in_pipe[1]);
        close(out_pipe[0]);
        close(out_pipe[1]);
        res = execve(cgi->getPath().data(), argv, env);
        exit(res);
    } else if (child_pid > 0) {
        freeEnv(env);
        close(in_pipe[0]);
        if (request->getContentLength() > 0) {
            cgi->setReqFd(in_pipe[1]);
        } else {
            close(in_pipe[1]);
        }
        close(out_pipe[1]);
        cgi->setPid(child_pid);
        cgi->setResFd(out_pipe[0]);
    } else {
        freeEnv(env);
        close(in_pipe[0]);
        close(in_pipe[1]);
        close(out_pipe[0]);
        close(out_pipe[1]);
    }
}

void HttpResponse::processGetRequest() {
    std::string absolute_path = this->request->getAbsolutPath();

    if (Path::isDirectory(absolute_path)) {
        if (this->location->getAutoindex()) {
            std::string uri    = this->request->getUriNoQuery();
            std::string html   = Path::getAutoIndexHtml(absolute_path, uri);
            long        ai_pos = static_cast<long>(Path::AUTOINDEX_HTML.find("<body>"));

            ai_pos += 6;
            this->body.assign(Path::AUTOINDEX_HTML.begin(), Path::AUTOINDEX_HTML.end());
            this->body.insert(this->body.begin() + ai_pos, html.begin(), html.end());
            this->body.insert(this->body.begin() + 66, uri.begin(), uri.end());
            this->body_size = this->body.size();
            this->setHeader("Content-Type", "text/html");
            this->setResponseString(HTTP_OK);
            return;
        }
        this->setError(HTTP_NOT_FOUND);
    } else {
        HTTPStatus error_code = this->writeFileToBuffer(absolute_path);

        if (error_code == HTTP_OK) {
            this->setResponseString(HTTP_OK);
        } else {
            this->setError(error_code);
        }
    }
}

void HttpResponse::processPostRequest() {
    if (this->server == nullptr) {
        this->setError(HTTP_BAD_REQUEST);
    } else {
        this->setResponseString(HTTP_OK);
        this->body_size = 0;
    }
}

void HttpResponse::processDeleteRequest() {
    std::string absolute_path = this->request->getAbsolutPath();

    if (Path::isNotEmptyDirectory(absolute_path)) {
        this->setError(HTTP_CONFLICT);
        return;
    }

    if (Path::fileExistsAndWritable(absolute_path) || Path::folderExistsAndWritable(absolute_path)) {
        if (std::remove(absolute_path.data()) != 0) {
            this->setError(HTTP_INTERNAL_SERVER_ERROR);
        } else {
            this->setResponseString(HTTP_OK);
        }
    } else {
        if (errno == EACCES) {
            this->setError(HTTP_FORBIDDEN);
        } else {
            this->setError(HTTP_GONE);
        }
    }
}

void HttpResponse::processPutRequest() {
    std::string absolute_path = this->request->getAbsolutPath();
    std::string file_name;

    file_name = Path::getFileNameFromPath(this->request->getRequestUri());

    if (!this->location->getFileUpload()) {
        this->setError(HTTP_METHOD_NOT_ALLOWED);
        return;
    }
    if (!Path::checkIfPathExists(absolute_path) || file_name.empty()) {
        this->setError(HTTP_NOT_FOUND);
        return;
    }
    if (Path::folderExistsAndWritable(absolute_path)) {
        this->setError(HTTP_CONFLICT);
        return;
    }
    if (Path::fileExistsAndWritable(absolute_path)) {
        this->setResponseString(HTTP_NO_CONTENT);
    } else {
        this->setResponseString(HTTP_CREATED);
    }

    std::ofstream rf(absolute_path, std::ios::out | std::ios::binary);
    if (rf) {
        rf.write(this->request->getBody().data(), static_cast<long >(this->request->getBody().size()));
        if (rf.bad()) {
            this->setError(HTTP_INTERNAL_SERVER_ERROR);
        }
        rf.close();
        this->setHeader("Content-Location", this->request->getRequestUri());
        return;
    } else {
        this->setError(HTTP_INTERNAL_SERVER_ERROR);
        return;
    }
}

HttpResponse::HTTPStatus HttpResponse::writeFileToBuffer(const std::string &file_path) {
    unsigned long length;
    std::ifstream file(file_path, std::ifstream::in | std::ifstream::binary);

    if (file) {
        file.seekg(0, file.end);
        length = file.tellg();
        if (length <= 0) {
            this->body_size = 0;
            return (HTTP_OK);
        }
        file.seekg(0, file.beg);
        this->body_size = (std::size_t) length;
        this->body.reserve(this->body.size() + length);
        this->body.insert(this->body.end(), std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
        if (file.peek() != EOF) {
            this->body_size = 0;
            return HTTP_INTERNAL_SERVER_ERROR;
        }
        file.close();
        this->setHeader("Content-Type", MimeType::getType(file_path));

        return HTTP_OK;
    } else {
        return HTTP_NOT_FOUND;
    }
}

void HttpResponse::setError(HTTPStatus code) {
    std::string error_code = std::to_string(code);
    std::string reason_phrase;
    std::string res;

    reason_phrase = HttpResponse::getReasonForStatus(code);
    std::string path;
    if (this->server != nullptr) {
        path = server->getCustomErrorPagePath(code);
    }
    this->setResponseString(code);
    if (code == HTTP_REQUEST_TIMEOUT) {
        this->setHeader("Connection", "close");
        headers.erase("Keep-Alive");
    }
    this->setHeader("Content-Type", "text/html");
    if (!path.empty()) {
        short i = this->writeFileToBuffer(path);
        if (i == 200) {
            return;
        }
    }
    res += "<!DOCTYPE html>\n<html><head><meta charset=\"utf-8\"/><title>" + error_code +
           "</title></head><body><h1>" + error_code + " " + reason_phrase + "</h1></body></html>\n";
    this->body.assign(res.begin(), res.end());
    this->body_size = this->body.size();
}

void HttpResponse::prepareData() {
    std::map<std::string, std::string>::iterator it  = headers.begin();
    time_t                                       now = time(nullptr);
    char                                         buf[100];

    this->setHeader("Content-Length", std::to_string(body_size));

    std::strftime(buf, 100, "%a, %d %b %Y %H:%M:%S %Z", gmtime(&now));
    this->setHeader("Date", buf);
    this->setHeader("Server", "webserv/1.0");

    if ((it = headers.find("Status")) != headers.end()) {
        headers.erase(it);
    }
    _headers_vec.reserve(500);
    _headers_vec.insert(_headers_vec.begin(), response_string.begin(), response_string.end());
    for (it = headers.begin(); it != headers.end(); ++it) {
        _headers_vec.insert(_headers_vec.end(), it->first.begin(), it->first.end());
        _headers_vec.push_back(':');
        _headers_vec.push_back(' ');
        _headers_vec.insert(_headers_vec.end(), it->second.begin(), it->second.end());
        _headers_vec.push_back('\r');
        _headers_vec.push_back('\n');
    }
    _headers_vec.push_back('\r');
    _headers_vec.push_back('\n');
}

int HttpResponse::send(int fd, size_t bytes) {
    size_t  pos_var = 0;
    ssize_t res     = 0;

    if (_headers_vec.empty()) {
        this->prepareData();
    }
    size_t to_send = 0;
    if (this->pos < _headers_vec.size()) {
        pos_var = this->pos;
        if (_headers_vec.size() - pos_var > bytes) {
            to_send = bytes;
        } else {
            to_send = _headers_vec.size() - pos_var;
        }
        res     = ::send(fd, _headers_vec.data() + pos_var, to_send, 0);
        if (res < 0 || (res == 0 && bytes > 0)) {
            return (-1);
        }
        this->pos += res;
        bytes -= res;
    }
    if (this->pos >= _headers_vec.size() &&
        !this->body.empty() && bytes > 0 &&
        this->body.size() > this->pos - _headers_vec.size()) {
        pos_var = this->pos - _headers_vec.size();
        if (this->body.size() - pos_var > bytes) {
            to_send = bytes;
        } else {
            to_send = this->body.size() - pos_var;
        }
        res     = ::send(fd, this->body.data() + pos_var, to_send, 0);
        if (res < 0 || (res == 0 && bytes > 0)) {
            return (-1);
        }
        this->pos += res;
    }
    if (this->pos == _headers_vec.size() + this->body.size()) {
        return 1;
    }
    return 0;
}

const std::string &HttpResponse::getReasonForStatus(HTTPStatus status) {
    switch (status) {
        case HTTP_CONTINUE:
            return HTTP_REASON_CONTINUE;
        case HTTP_SWITCHING_PROTOCOLS:
            return HTTP_REASON_SWITCHING_PROTOCOLS;
        case HTTP_PROCESSING:
            return HTTP_REASON_PROCESSING;
        case HTTP_OK:
            return HTTP_REASON_OK;
        case HTTP_CREATED:
            return HTTP_REASON_CREATED;
        case HTTP_ACCEPTED:
            return HTTP_REASON_ACCEPTED;
        case HTTP_NONAUTHORITATIVE:
            return HTTP_REASON_NONAUTHORITATIVE;
        case HTTP_NO_CONTENT:
            return HTTP_REASON_NO_CONTENT;
        case HTTP_RESET_CONTENT:
            return HTTP_REASON_RESET_CONTENT;
        case HTTP_PARTIAL_CONTENT:
            return HTTP_REASON_PARTIAL_CONTENT;
        case HTTP_MULTI_STATUS:
            return HTTP_REASON_MULTI_STATUS;
        case HTTP_ALREADY_REPORTED:
            return HTTP_REASON_ALREADY_REPORTED;
        case HTTP_IM_USED:
            return HTTP_REASON_IM_USED;
        case HTTP_MULTIPLE_CHOICES:
            return HTTP_REASON_MULTIPLE_CHOICES;
        case HTTP_MOVED_PERMANENTLY:
            return HTTP_REASON_MOVED_PERMANENTLY;
        case HTTP_FOUND:
            return HTTP_REASON_FOUND;
        case HTTP_SEE_OTHER:
            return HTTP_REASON_SEE_OTHER;
        case HTTP_NOT_MODIFIED:
            return HTTP_REASON_NOT_MODIFIED;
        case HTTP_USE_PROXY:
            return HTTP_REASON_USE_PROXY;
        case HTTP_TEMPORARY_REDIRECT:
            return HTTP_REASON_TEMPORARY_REDIRECT;
        case HTTP_BAD_REQUEST:
            return HTTP_REASON_BAD_REQUEST;
        case HTTP_UNAUTHORIZED:
            return HTTP_REASON_UNAUTHORIZED;
        case HTTP_PAYMENT_REQUIRED:
            return HTTP_REASON_PAYMENT_REQUIRED;
        case HTTP_FORBIDDEN:
            return HTTP_REASON_FORBIDDEN;
        case HTTP_NOT_FOUND:
            return HTTP_REASON_NOT_FOUND;
        case HTTP_METHOD_NOT_ALLOWED:
            return HTTP_REASON_METHOD_NOT_ALLOWED;
        case HTTP_NOT_ACCEPTABLE:
            return HTTP_REASON_NOT_ACCEPTABLE;
        case HTTP_PROXY_AUTHENTICATION_REQUIRED:
            return HTTP_REASON_PROXY_AUTHENTICATION_REQUIRED;
        case HTTP_REQUEST_TIMEOUT:
            return HTTP_REASON_REQUEST_TIMEOUT;
        case HTTP_CONFLICT:
            return HTTP_REASON_CONFLICT;
        case HTTP_GONE:
            return HTTP_REASON_GONE;
        case HTTP_LENGTH_REQUIRED:
            return HTTP_REASON_LENGTH_REQUIRED;
        case HTTP_PRECONDITION_FAILED:
            return HTTP_REASON_PRECONDITION_FAILED;
        case HTTP_REQUEST_ENTITY_TOO_LARGE:
            return HTTP_REASON_REQUEST_ENTITY_TOO_LARGE;
        case HTTP_REQUEST_URI_TOO_LONG:
            return HTTP_REASON_REQUEST_URI_TOO_LONG;
        case HTTP_UNSUPPORTED_MEDIA_TYPE:
            return HTTP_REASON_UNSUPPORTED_MEDIA_TYPE;
        case HTTP_REQUESTED_RANGE_NOT_SATISFIABLE:
            return HTTP_REASON_REQUESTED_RANGE_NOT_SATISFIABLE;
        case HTTP_EXPECTATION_FAILED:
            return HTTP_REASON_EXPECTATION_FAILED;
        case HTTP_IM_A_TEAPOT:
            return HTTP_REASON_IM_A_TEAPOT;
        case HTTP_ENCHANCE_YOUR_CALM:
            return HTTP_REASON_ENCHANCE_YOUR_CALM;
        case HTTP_MISDIRECTED_REQUEST:
            return HTTP_REASON_MISDIRECTED_REQUEST;
        case HTTP_UNPROCESSABLE_ENTITY:
            return HTTP_REASON_UNPROCESSABLE_ENTITY;
        case HTTP_LOCKED:
            return HTTP_REASON_LOCKED;
        case HTTP_FAILED_DEPENDENCY:
            return HTTP_REASON_FAILED_DEPENDENCY;
        case HTTP_UPGRADE_REQUIRED:
            return HTTP_REASON_UPGRADE_REQUIRED;
        case HTTP_PRECONDITION_REQUIRED:
            return HTTP_REASON_PRECONDITION_REQUIRED;
        case HTTP_TOO_MANY_REQUESTS:
            return HTTP_REASON_TOO_MANY_REQUESTS;
        case HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE:
            return HTTP_REASON_REQUEST_HEADER_FIELDS_TOO_LARGE;
        case HTTP_UNAVAILABLE_FOR_LEGAL_REASONS:
            return HTTP_REASON_UNAVAILABLE_FOR_LEGAL_REASONS;
        case HTTP_INTERNAL_SERVER_ERROR:
            return HTTP_REASON_INTERNAL_SERVER_ERROR;
        case HTTP_NOT_IMPLEMENTED:
            return HTTP_REASON_NOT_IMPLEMENTED;
        case HTTP_BAD_GATEWAY:
            return HTTP_REASON_BAD_GATEWAY;
        case HTTP_SERVICE_UNAVAILABLE:
            return HTTP_REASON_SERVICE_UNAVAILABLE;
        case HTTP_GATEWAY_TIMEOUT:
            return HTTP_REASON_GATEWAY_TIMEOUT;
        case HTTP_VERSION_NOT_SUPPORTED:
            return HTTP_REASON_VERSION_NOT_SUPPORTED;
        case HTTP_VARIANT_ALSO_NEGOTIATES:
            return HTTP_REASON_VARIANT_ALSO_NEGOTIATES;
        case HTTP_INSUFFICIENT_STORAGE:
            return HTTP_REASON_INSUFFICIENT_STORAGE;
        case HTTP_LOOP_DETECTED:
            return HTTP_REASON_LOOP_DETECTED;
        case HTTP_NOT_EXTENDED:
            return HTTP_REASON_NOT_EXTENDED;
        case HTTP_NETWORK_AUTHENTICATION_REQUIRED:
            return HTTP_REASON_NETWORK_AUTHENTICATION_REQUIRED;
        default:
            return HTTP_REASON_UNKNOWN;
    }
}


const std::string HttpResponse::HTTP_REASON_CONTINUE                        = "Continue";
const std::string HttpResponse::HTTP_REASON_SWITCHING_PROTOCOLS             = "Switching Protocols";
const std::string HttpResponse::HTTP_REASON_PROCESSING                      = "Processing";
const std::string HttpResponse::HTTP_REASON_OK                              = "OK";
const std::string HttpResponse::HTTP_REASON_CREATED                         = "Created";
const std::string HttpResponse::HTTP_REASON_ACCEPTED                        = "Accepted";
const std::string HttpResponse::HTTP_REASON_NONAUTHORITATIVE                = "Non-Authoritative Information";
const std::string HttpResponse::HTTP_REASON_NO_CONTENT                      = "No Content";
const std::string HttpResponse::HTTP_REASON_RESET_CONTENT                   = "Reset Content";
const std::string HttpResponse::HTTP_REASON_PARTIAL_CONTENT                 = "Partial Content";
const std::string HttpResponse::HTTP_REASON_MULTI_STATUS                    = "Multi Status";
const std::string HttpResponse::HTTP_REASON_ALREADY_REPORTED                = "Already Reported";
const std::string HttpResponse::HTTP_REASON_IM_USED                         = "IM Used";
const std::string HttpResponse::HTTP_REASON_MULTIPLE_CHOICES                = "Multiple Choices";
const std::string HttpResponse::HTTP_REASON_MOVED_PERMANENTLY               = "Moved Permanently";
const std::string HttpResponse::HTTP_REASON_FOUND                           = "Found";
const std::string HttpResponse::HTTP_REASON_SEE_OTHER                       = "See Other";
const std::string HttpResponse::HTTP_REASON_NOT_MODIFIED                    = "Not Modified";
const std::string HttpResponse::HTTP_REASON_USE_PROXY                       = "Use Proxy";
const std::string HttpResponse::HTTP_REASON_TEMPORARY_REDIRECT              = "Temporary Redirect";
const std::string HttpResponse::HTTP_REASON_PERMANENT_REDIRECT              = "Permanent Redirect";
const std::string HttpResponse::HTTP_REASON_BAD_REQUEST                     = "Bad Request";
const std::string HttpResponse::HTTP_REASON_UNAUTHORIZED                    = "Unauthorized";
const std::string HttpResponse::HTTP_REASON_PAYMENT_REQUIRED                = "Payment Required";
const std::string HttpResponse::HTTP_REASON_FORBIDDEN                       = "Forbidden";
const std::string HttpResponse::HTTP_REASON_NOT_FOUND                       = "Not Found";
const std::string HttpResponse::HTTP_REASON_METHOD_NOT_ALLOWED              = "Method Not Allowed";
const std::string HttpResponse::HTTP_REASON_NOT_ACCEPTABLE                  = "Not Acceptable";
const std::string HttpResponse::HTTP_REASON_PROXY_AUTHENTICATION_REQUIRED   = "Proxy Authentication Required";
const std::string HttpResponse::HTTP_REASON_REQUEST_TIMEOUT                 = "Request Time-out";
const std::string HttpResponse::HTTP_REASON_CONFLICT                        = "Conflict";
const std::string HttpResponse::HTTP_REASON_GONE                            = "Gone";
const std::string HttpResponse::HTTP_REASON_LENGTH_REQUIRED                 = "Length Required";
const std::string HttpResponse::HTTP_REASON_PRECONDITION_FAILED             = "Precondition Failed";
const std::string HttpResponse::HTTP_REASON_REQUEST_ENTITY_TOO_LARGE        = "Request Entity Too Large";
const std::string HttpResponse::HTTP_REASON_REQUEST_URI_TOO_LONG            = "Request-URI Too Large";
const std::string HttpResponse::HTTP_REASON_UNSUPPORTED_MEDIA_TYPE          = "Unsupported Media Type";
const std::string HttpResponse::HTTP_REASON_REQUESTED_RANGE_NOT_SATISFIABLE = "Requested Range Not Satisfiable";
const std::string HttpResponse::HTTP_REASON_EXPECTATION_FAILED              = "Expectation Failed";
const std::string HttpResponse::HTTP_REASON_IM_A_TEAPOT                     = "I'm a Teapot";
const std::string HttpResponse::HTTP_REASON_ENCHANCE_YOUR_CALM              = "Enchance Your Calm";
const std::string HttpResponse::HTTP_REASON_MISDIRECTED_REQUEST             = "Misdirected Request";
const std::string HttpResponse::HTTP_REASON_UNPROCESSABLE_ENTITY            = "Unprocessable Entity";
const std::string HttpResponse::HTTP_REASON_LOCKED                          = "Locked";
const std::string HttpResponse::HTTP_REASON_FAILED_DEPENDENCY               = "Failed Dependency";
const std::string HttpResponse::HTTP_REASON_UPGRADE_REQUIRED                = "Upgrade Required";
const std::string HttpResponse::HTTP_REASON_PRECONDITION_REQUIRED           = "Precondition Required";
const std::string HttpResponse::HTTP_REASON_TOO_MANY_REQUESTS               = "Too Many Requests";
const std::string HttpResponse::HTTP_REASON_REQUEST_HEADER_FIELDS_TOO_LARGE = "Request Header Fields Too Large";
const std::string HttpResponse::HTTP_REASON_UNAVAILABLE_FOR_LEGAL_REASONS   = "Unavailable For Legal Reasons";
const std::string HttpResponse::HTTP_REASON_INTERNAL_SERVER_ERROR           = "Internal Server Error";
const std::string HttpResponse::HTTP_REASON_NOT_IMPLEMENTED                 = "Not Implemented";
const std::string HttpResponse::HTTP_REASON_BAD_GATEWAY                     = "Bad Gateway";
const std::string HttpResponse::HTTP_REASON_SERVICE_UNAVAILABLE             = "Service Unavailable";
const std::string HttpResponse::HTTP_REASON_GATEWAY_TIMEOUT                 = "Gateway Time-Out";
const std::string HttpResponse::HTTP_REASON_VERSION_NOT_SUPPORTED           = "HTTP Version Not Supported";
const std::string HttpResponse::HTTP_REASON_VARIANT_ALSO_NEGOTIATES         = "Variant Also Negotiates";
const std::string HttpResponse::HTTP_REASON_INSUFFICIENT_STORAGE            = "Insufficient Storage";
const std::string HttpResponse::HTTP_REASON_LOOP_DETECTED                   = "Loop Detected";
const std::string HttpResponse::HTTP_REASON_NOT_EXTENDED                    = "Not Extended";
const std::string HttpResponse::HTTP_REASON_NETWORK_AUTHENTICATION_REQUIRED = "Network Authentication Required";
const std::string HttpResponse::HTTP_REASON_UNKNOWN                         = "???";

Cgi *HttpResponse::getCgi() const {
    return cgi;
}

void HttpResponse::setCgi(Cgi *_cgi) {
    HttpResponse::cgi = _cgi;
}

void HttpResponse::writeToCgi(HttpRequest *req, size_t bytes) {
    int    res;
    size_t size;
    size_t position = this->cgi->getPos();

    size = req->getBody().size() - position;
    if (req->getBody().size() - position > bytes) {
        size = bytes;
    }
    res  = write(this->cgi->getReqFd(), req->getBody().data() + position, size);
    if (res > 0) {
        this->cgi->setPos(position + res);
    } else if (req->getBody().size() - position > 0 && res == 0) {
        return;
    } else if (position == req->getBody().size()) {
//        this->cgi->setPos(0);
    } else {
        this->setError(HTTP_INTERNAL_SERVER_ERROR);
    }
}

bool HttpResponse::readCgi(size_t bytes, bool eof) {
    char                   buff[CGI_BUF_SIZE];
    int                    res;
    int                    fd = this->cgi->getResFd();
    std::string::size_type position;

    res = read(fd, &buff, bytes);
    if (res < 0) {
        return false;
    }
    this->body.append(buff, bytes);
    if (!this->cgi->isHeadersParsed() && (position = this->body.find("\r\n\r\n")) != std::string::npos) {
        if (!this->parseCgiHeaders(position)) {
            this->setError(HTTP_INTERNAL_SERVER_ERROR);
            close(fd);
            return false;
        }
        this->cgi->setHeadersParsed(true);
        this->body.erase(body.begin(), this->body.begin() + position + 4);
    }
    if (eof || res == 0) {
        body_size = body.size();
        this->setHeader("Content-Length", std::to_string(body_size));
        this->setResponseString(HTTP_OK);
        return true;
    }
    return false;
}

bool HttpResponse::checkCgiHeaders(size_t &i, const std::string &sep, size_t max, std::string &token) {
    i = this->body.find_first_not_of(" \t\r\n", i);
    if (i == std::string::npos) {
        return false;
    }
    std::size_t end = this->body.find_first_of("\r\n", i);
    if (end == std::string::npos) {
        end = this->body.length();
    }
    std::size_t next = body.find_first_of(sep, i);
    if (next == std::string::npos) {
        if (sep == ":") {
            return false;
        }
        next = end;
    }
    token            = this->body.substr(i, next - i);
    if (token.empty() || token.length() > max) {
        return false;
    }
    i = next;
    return true;
}

bool HttpResponse::parseCgiHeaders(size_t end) {
    std::string name;
    std::string value;
    size_t      i = 0;

    while (i < end && body.find("\r\n\r\n", i) != i) {
        if (!this->checkCgiHeaders(i, ":", HttpRequest::MAX_NAME, name) ||
            !this->checkCgiHeaders(++i, "\r\n", HttpRequest::MAX_VALUE, value)) {
            break;
        }
        setHeader(name, value);
    }
    if (i != end) {
        return false;
    }
    return true;
}

//const std::string HttpResponse::DATE                                        = "Date";
//const std::string HttpResponse::SET_COOKIE                                  = "Set-Cookie";
