#ifndef WEBSERV_LOCATION_HPP
#define WEBSERV_LOCATION_HPP

# include <string>
# include <vector>
# include "IDirective.hpp"

class Location: public IDirective
{
private:
	std::string       	_path;
	std::string       	_root;
	std::vector<bool> 	_methods;
	bool              	_file_upload;
	std::string			_upload_tmp_path;
	std::string       	_index;
	bool              	_autoindex;
	std::string			_cgi_pass;
	std::string         _redirection;

	Location(Location const &a);
	Location &operator=(Location const &a);
public:
	Location();
	~Location();

	std::string getPath() const;
	std::string getRoot() const;
	std::vector<bool> getMethods() const;
	bool getFileUpload() const;
	std::string getUploadTmpPath() const;
	std::string getIndex() const;
	bool getAutoindex() const;
	std::string getCgiPass() const;
	std::string getRedirection() const;

	void setPath(std::string const &path);
	void setRoot(std::string const &root);
	void setMethods(std::vector<bool> methods);
	void setMethod(bool method);
	void setMethod(unsigned int index, bool method);
	void setFileUpload(bool file_upload);
	void setUploadTmpPath(std::string const &upload_tmp_path);
	void setIndex(std::string const &index);
	void setAutoindex(bool autoindex);
	void setCgiPass(std::string const &cgi_pass);
	void setRedirection(std::string const &redirection);

	bool methodAllowed(const std::string &method) const;
	std::string getAllowedMethodsField() const;
};


#endif //WEBSERV_LOCATION_HPP
