#include "Config.hpp"

void (Config::*parseDirective[15])(
    IDirective *server,
    Config::iterator &value
) = {
    &Config::parseListen,
    &Config::parsePort,
    &Config::parseServerName,
    &Config::parseClientMaxBodySize,
    &Config::parseMimeConfPath,
    &Config::parseErrorPages,
    &Config::parseLocation,
    &Config::parseRedirection,
    &Config::parseRoot,
    &Config::parseMethods,
    &Config::parseFileUpload,
    &Config::parseUploadTmpPath,
    &Config::parseIndex,
    &Config::parseAutoindex,
    &Config::parseCgiPass
};

Config::Config(const std::string &path_to_file)
: _path_to_file(path_to_file), _config()
{
    if (access(path_to_file.c_str(), F_OK) == -1)
        throw Config::ConfigException("config file " + path_to_file + " not found");
    this->parse(path_to_file);
}

Config::~Config()
{
    std::vector<Server *>::iterator s_it = _servers.begin();

    while (s_it != _servers.end())
    {
        delete *s_it;
        ++s_it;
    }
    _servers.clear();
}

void Config::parse(const std::string &path_to_file)
{
    std::string line;
    std::ifstream file(path_to_file.c_str());
    std::string token;

    if (!file.is_open())
    {
        throw Config::ConfigException("config file " + path_to_file + " is locked");
    }

    while (std::getline(file, line))
    {
        // skip comment and empty line
        if (!line.length() || isComment(line))
        {
            continue;
        }
        // clear comment in the end
        line = line.substr(0, line.find("#", 0));
        // split by witespace
        std::istringstream iss(line);
        std::vector<std::string> tokens(
            (std::istream_iterator<std::string>(iss)),
            std::istream_iterator<std::string>());
        iterator token;
        // check trash derective
        if (tokens.size() > 0)
        {
            this->checkTrashDirective(tokens[0]);
        }
        for (token = tokens.begin(); token != tokens.end(); ++token)
        {
            std::string t = *token;

            if (t.length() > 1 && isDirectiveLine(t))
            {
                // delete last chr
                _config.push_back(t.erase(t.size() - 1));
                _config.push_back(";");
            }
            else
            {
                _config.push_back(*token);
            }
        }
    }
    file.close();
    this->parseServerData(_config.begin());
}

void Config::parseServerData(iterator value)
{
    if (value == _config.end())
    {
        return ;
    }
    if (value == _config.end() || *(value++) != "server")
    {
        throw Config::ConfigException(
            "invalid number of arguments in server directive in" + _path_to_file
        );   
    }
    if (value == _config.end() || *(value++) != "{")
    {
        throw Config::ConfigException(
            "invalid number of arguments in server directive in" + _path_to_file
        );
    }

    Server *server = new Server();

    while (value != _config.end() && *value != "}")
    {
        try
        {
            (this->*parseDirective[_getDirectIndex(_directs, *value)])(server, ++value);
        } 
        catch(char const *error)
        {
            delete server;
            throw Config::ConfigException(
                "error in server directive: " + std::string(error)  + 
                " in " + _path_to_file
            );
        }
        catch(std::string &error)
        {
            delete server;
            throw Config::ConfigException(
                "error in server directive: expected \';\' near " + 
                error + " in " + _path_to_file
            );
        }
        if (myNext(value) == _config.end())
        {
            delete server;
            throw Config::ConfigException("incorrect syntax in " + _path_to_file);
        }
        value++;
    }
    if (value == _config.end() || *value != "}")
    {
        delete server;
        throw Config::ConfigException("incorrect syntax in " + _path_to_file);   
    }
    this->setDefault(server);
    _servers.push_back(server);
    this->parseServerData(++value);
}

const std::vector<Server *> &Config::getServers() const
{
    return _servers;
}

in_addr_t Config::getDefaultHost() const
{
    return _default_host;
}

uint16_t Config::getDefaultPort() const
{
    return _default_port;
}

Config::ConfigException::ConfigException()
: msg("")
{}

Config::ConfigException::ConfigException(const std::string &msg)
: msg("webserv: [error] " + msg)
{}

Config::ConfigException::~ConfigException() throw()
{}

const char *Config::ConfigException::what() const throw()
{
    return msg.c_str();
}

bool Config::isComment(std::string const &str) const
{
    std::size_t found = str.find_first_not_of(" \t\f\v\n\r");

    if (found != std::string::npos)
    {
        if (str.at(found) == '#')
        {
            return true;
        }
    }
    return false;
}

bool Config::isDirectiveLine(std::string const &str) const
{
    std::size_t found = str.find_last_not_of(" \t\f\v\n\r");
    
    if (found != std::string::npos)
    {
        if (str.at(found) == ';')
        {
            return true;
        }
    }
    return false;
}

void Config::checkTrashDirective(std::string const &name) const
{
    if (name != "{" && name != "}" && 
        name != ";" && name != "server")
    {
        try
        {
            _getDirectIndex(_directs, name);
        }
        catch (const char *error)
        {
            throw Config::ConfigException(
                "unknown directive " + name + " in " + _path_to_file
            );
        }
    }
}

void Config::parseListen(IDirective *server, iterator &value)
{
    if (value == _config.end())
    {
        throw "not correct directive in listen";
    }
    if (!dynamic_cast<Server *>(server))
    {
        throw "\'listen\' directive supported in server only";
    }
    ((Server *)server)->setHost(inet_addr((*value).data()));
    if (!(myNext(value) != _config.end() && *(myNext(value)) == ";"))
    {
        throw std::string(*(myPrev(value)) + " " + *value + " ->;");
    }
    value++;
}

void Config::parsePort(IDirective *server, iterator &value)
{
    if (value == _config.end())
    {
        throw "not correct directive in port";
    }
    if (!dynamic_cast<Server *>(server))
    {
        throw "\'port\' directive supported in server only";
    }
    ((Server *)server)->setPort((uint16_t) atoll((*value).c_str()));
    if (!(myNext(value) != _config.end() && *(myNext(value)) == ";"))
    {
        throw std::string(*(myPrev(value)) + " " + *value + " ->;");
    }
    value++;
}

void Config::parseServerName(IDirective *server, iterator &value)
{
    if (value == _config.end())
    {
        throw "not correct directive in server_name";
    }
    if (!dynamic_cast<Server *>(server))
    {
        throw "\'server_name\' directive supported in server only";
    }
    ((Server *)server)->setServerName(*value);
    if (!(myNext(value) != _config.end() && *(myNext(value)) == ";"))
    {
        throw std::string(*(myPrev(value)) + " " + *value + " ->;");
    }
    value++;
}

void Config::parseClientMaxBodySize(IDirective *server, iterator &value)
{
    if (value == _config.end())
    {
        throw "not correct directive in client_max_body_size";
    }
    if (!dynamic_cast<Server *>(server))
    {
        throw "\'client_max_body_size\' directive supported in server only";
    }
    ((Server *)server)->setClientMaxBodySize(atoll((*value).c_str()));
    if (!(myNext(value) != _config.end() && *(myNext(value)) == ";"))
    {
        throw std::string(*(myPrev(value)) + " " + *value + " ->;");
    }
    value++;
}

void Config::parseMimeConfPath(IDirective *server, iterator &value)
{
    if (value == _config.end())
    {
        throw "not correct directive in mime_conf_path";
    }
    if (!dynamic_cast<Server *>(server))
    {
        throw "\'mime_conf_path\' directive supported in server only";
    }
    ((Server *)server)->setMimeConfPath(*value);
    if (!(myNext(value) != _config.end() && *(myNext(value)) == ";"))
    {
        throw std::string(*(myPrev(value)) + " " + *value + " ->;");
    }
    value++;
}

void Config::parseErrorPages(IDirective *server, iterator &value)
{
    if (value == _config.end())
    {
        throw "not correct directive in error_page";
    }
    if (!dynamic_cast<Server *>(server))
    {
        throw "\'error_page\' directive supported in server only";
    }
    ((Server *)server)->setErrorPage(std::make_pair(
        (short) std::atoi((*value).c_str()), 
        *(++value))
    );
    if (!(myNext(value) != _config.end() && *(myNext(value)) == ";"))
    {
        throw std::string(*(myPrev(value)) + " " + *value + " ->;");
    }
    value++;
}

void Config::parseLocation(IDirective *server, iterator &value)
{
    std::string path = *value;

    if (++value == _config.end() || *(value++) != "{")
    {
        throw Config::ConfigException(
            "invalid number of arguments in location directive in " + _path_to_file
        );
    }

    Location *location = new Location();
    
    while (value != _config.end() && *value != "}")
    {
        try
        {
            (this->*parseDirective[_getDirectIndex(_directs, *value)])(location, ++value);
        } 
        catch(char const *error)
        {
            delete location;
            throw Config::ConfigException(
                "error in location directive: " + std::string(error)  + 
                " in " + _path_to_file
            );
        }
        catch(std::string &error)
        {
            delete server;
            throw Config::ConfigException(
                "error in location directive: expected \';\' near " + 
                error + " in " + _path_to_file
            );
        }
        if (myNext(value) == _config.end())
        {
            delete location;
            throw Config::ConfigException("incorrect syntax in " + _path_to_file);
        }
        value++;
    }

    location->setPath(path);
    ((Server *)server)->setLocation(location);
}

void Config::parseRedirection(IDirective *server, iterator &value)
{
    if (value == _config.end())
    {
        throw "not correct directive in return";
    }
    if (!dynamic_cast<Location *>(server))
    {
        throw "\'return\' directive supported in location only";
    }
    ((Location *)server)->setRedirection(*value);
    if (!(myNext(value) != _config.end() && *(myNext(value)) == ";"))
    {
        throw std::string(*(myPrev(value)) + " " + *value + " ->;");
    }
    value++;
}

void Config::parseRoot(IDirective *server, iterator &value)
{
    if (value == _config.end())
    {
        throw "not correct directive in root";
    }
    if (!dynamic_cast<Location *>(server))
    {
        throw "\'root\' directive supported in location only";
    }
    ((Location *)server)->setRoot(*value);
    if (!(myNext(value) != _config.end() && *(myNext(value)) == ";"))
    {
        throw std::string(*(myPrev(value)) + " " + *value + " ->;");
    }
    value++;
}

void Config::parseMethods(IDirective *server, iterator &value)
{
    if (value == _config.end())
    {
        throw "not correct directive in methods";
    }
    if (!dynamic_cast<Location *>(server))
    {
        throw "\'methods\' directive supported in location only";
    }
    ((Location *)server)->setMethod(0, false);
    while (value != _config.end())
    {
        ((Location *)server)->setMethod(_getMethodIndex(_methods, *value), true);
        if (*(myNext(value)) == ";")
        {
            break;
        }
        value++;
    }
    if (!(myNext(value) != _config.end() && *(myNext(value)) == ";"))
    {
        throw std::string(*(myPrev(value)) + " " + *value + " ->;");
    }
    value++;
}

void Config::parseFileUpload(IDirective *server, iterator &value)
{
    if (value == _config.end() || !(*value == "on" || *value == "off"))
    {
        throw "not correct directive in file_upload";
    }
    if (!dynamic_cast<Location *>(server))
    {
        throw "\'file_upload\' directive supported in location only";
    }
    ((Location *)server)->setFileUpload(*value == "on" ? true : false);
    if (!(myNext(value) != _config.end() && *(myNext(value)) == ";"))
    {
        throw std::string(*(myPrev(value)) + " " + *value + " ->;");
    }
    value++;
}

void Config::parseUploadTmpPath(IDirective *server, iterator &value)
{
    if (value == _config.end())
    {
        throw "not correct directive in upload_tmp_path";
    }
    if (!dynamic_cast<Location *>(server))
    {
        throw "\'upload_tmp_path\' directive supported in location only";
    }
    ((Location *)server)->setUploadTmpPath(*value);
    if (!(myNext(value) != _config.end() && *(myNext(value)) == ";"))
    {
        throw std::string(*(myPrev(value)) + " " + *value + " ->;");
    }
    value++;
}

void Config::parseIndex(IDirective *server, iterator &value)
{
    if (value == _config.end())
    {
        throw "not correct directive in index";
    }
    if (!dynamic_cast<Location *>(server))
    {
        throw "\'index\' directive supported in location only";
    }
    ((Location *)server)->setIndex(*value);
    if (!(myNext(value) != _config.end() && *(myNext(value)) == ";"))
    {
        throw std::string(*(myPrev(value)) + " " + *value + " ->;");
    }
    value++;
}

void Config::parseAutoindex(IDirective *server, iterator &value)
{
    if (value == _config.end() || !(*value == "on" || *value == "off"))
    {
        throw "not correct directive in autoindex";
    }
    if (!dynamic_cast<Location *>(server))
    {
        throw "\'autoindex\' directive supported in location only";
    }
    ((Location *)server)->setAutoindex(*value == "on" ? true : false);
    if (!(myNext(value) != _config.end() && *(myNext(value)) == ";"))
    {
        throw std::string(*(myPrev(value)) + " " + *value + " ->;");
    }
    value++;
}

void Config::parseCgiPass(IDirective *server, iterator &value)
{
    if (value == _config.end())
    {
        throw "not correct directive in cgi_pass";
    }
    if (!dynamic_cast<Location *>(server))
    {
        throw "\'cgi_pass\' directive supported in location only";
    }
    ((Location *)server)->setCgiPass(*value);
    if (!(myNext(value) != _config.end() && *(myNext(value)) == ";"))
    {
        throw std::string(*(myPrev(value)) + " " + *value + " ->;");
    }
    value++;
}

void Config::setDefault(Server *server)
{
    if (!_servers.size())
    {
        _default_host = server->getHost();
        _default_port = server->getPort();
    }
}
