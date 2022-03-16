#include "HttpRequest.hpp"


HttpRequest::HttpRequest()
        :
        method(),
        request_uri(),
        query_string(),
        absolute_path(),
        http_v("HTTP/1.1"),
        chunked(false),
        content_length(0),
        ready(false),
        parsing_error(HttpResponse::HTTP_OK) {}

void HttpRequest::setReady(bool status) {
    this->ready = status;
}

bool HttpRequest::getReady() const {
    return this->ready;
}

const std::string &HttpRequest::getRequestUri() const {
    return this->request_uri;
}

uint16_t HttpRequest::getParsingError() const {
    return this->parsing_error;
}

void HttpRequest::setParsingError(uint16_t status) {
    this->parsing_error = status;
}

unsigned long HttpRequest::getContentLength() const {
    return this->content_length;
}

void HttpRequest::setContentLength(unsigned long len) {
    this->content_length = len;
}

bool HttpRequest::getChunked() const {
    return this->chunked;
}

const std::string &HttpRequest::getUriNoQuery() const {
    return this->uri_no_query;
}

const std::string &HttpRequest::getMethod() const {
    return this->method;
}

const std::string &HttpRequest::getAbsolutPath() const {
    return this->absolute_path;
}

void HttpRequest::setAbsolutPath(const std::string &path) {
    this->absolute_path = path;
}

bool HttpRequest::headersSent(const std::string &req) {
    if (this->request_uri.empty()) {
        if (req.find("\r\n\r\n") == std::string::npos) {
            if (req.size() > MAX_MESSAGE) {
                this->setParsingError(HttpResponse::HTTP_BAD_REQUEST);
                this->setReady(true);
                return true;
            }
            return false;
        }
        return true;
    }
    return false;
}

bool HttpRequest::parseRequestLine(const std::string &request, size_t &pos) {
    if (!this->parse(request, pos, " ", true, MAX_METHOD, this->method)) {
        this->parsing_error = HttpResponse::HTTP_METHOD_NOT_ALLOWED;
        return false;
    }

    if (!this->parse(request, pos, " ", true, MAX_URI, this->request_uri)) {
        this->parsing_error = HttpResponse::HTTP_REQUEST_URI_TOO_LONG;
        return false;
    }
    this->request_uri = Path::urlDecode(this->request_uri);

    if (!this->parse(request, pos, "\r\n", false, MAX_V, this->http_v) ||
        (this->http_v != "HTTP/1.1" && this->http_v != "HTTP/1.0")) {
        this->parsing_error = HttpResponse::HTTP_VERSION_NOT_SUPPORTED;
        return false;
    }

    return true;
}

bool HttpRequest::parseHeaders(const std::string &buffer, size_t &pos) {
    std::string key;
    std::string value;
    int         num = 0;

    while (pos < buffer.size() && buffer.find("\r\n\r\n", pos) != pos) {
        if (num > MAX_HEADERS) {
            this->parsing_error = HttpResponse::HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE;
            return false;
        }

        if (!this->parse(buffer, pos, ":", true, MAX_NAME, key)) {
            this->parsing_error = HttpResponse::HTTP_BAD_REQUEST;
            return false;
        }

        if (!this->parse(buffer, ++pos, "\r\n", false, MAX_VALUE, value)) {
            this->parsing_error = HttpResponse::HTTP_BAD_REQUEST;
            return false;
        }

        this->setHeader(key, value);
        ++num;
    }

    if (buffer.find("\r\n\r\n", pos) == pos) {
        pos += 4;
        return true;
    }

    return false;
}

bool HttpRequest::processUri() {
    size_t res = this->request_uri.find("?");

    if (res == std::string::npos) {
        this->query_string = "";
        this->uri_no_query = this->request_uri;
    } else {
        this->query_string = this->request_uri.substr(res + 1, this->request_uri.size());
        this->uri_no_query = this->request_uri.substr(0, res);
    }

    return true;
}

bool HttpRequest::processHeaders() {
    std::map<std::string, std::string>::iterator it;

    it = this->headers.find("Transfer-Encoding");

    if (it == this->headers.end()) {
        it            = this->headers.find("Content-Length");

        if (it != this->headers.end()) {
            this->content_length = std::strtoul(it->second.data(), nullptr, 10);
            if (errno == ERANGE) {
                this->parsing_error = HttpResponse::HTTP_REQUEST_ENTITY_TOO_LARGE;
                return false;
            }
        } else if ((this->method == "POST" || this->method == "PUT") &&
                   this->headers.find("Content-Length") == this->headers.end() && !this->getChunked()) {
            this->parsing_error = HttpResponse::HTTP_LENGTH_REQUIRED;
        }
        this->chunked = false;
    } else if (it->second.find("chunked") != std::string::npos) {
        this->chunked = true;
    }

    if (this->parsing_error != HttpResponse::HTTP_OK) {
        return false;
    }

    if (this->method == "DELETE" || this->method == "GET") {
        this->body.clear();
        this->content_length = 0;
        this->setReady(true);
        return true;
    }
    return true;
}

bool HttpRequest::checkContentLength(unsigned long size) {
    if (this->content_length > size) {
        this->parsing_error = HttpResponse::HTTP_REQUEST_ENTITY_TOO_LARGE;
    }
    return true;
}

const std::string &HttpRequest::getQueryString() const {
    return query_string;
}
