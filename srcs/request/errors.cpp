// Copyright 2024 42Madrid GPL
// Author: alvjimen
#include "Request.hpp"
#include "cgimanager.hpp"

void request::Handler::debug_Location(parser::Location *location)
{
  std::cout << "getURI " << location->getURI() << std::endl;
  std::cout << "getDirectoryListing: " << location->getAutoindex() << std::endl;
  std::cout << "getMBS: " << location->getCMBS() << std::endl;
  std::cout << "getRoot: " << location->getRoot() << std::endl;
  std::cout << "getAlias: " << location->getAlias() << std::endl;
  std::cout << "getReturn(second):" << location->getReturn().second << std::endl;
  std::cout << "getIndex: " << std::endl;
  sprintVector(location->getIndex());
  std::cout << "getAcceptMethod:" << std::endl;
  sprintVector(location->getAcceptMethod());
  std::cout << "getUploadPass: " << location->getUploadPass() << std::endl;
  std::cout << "getUploadStore: " << location->getUploadStore() << std::endl;
  std::cout << "getCgiPass(size): " <<  location->getCgiPass().size() << std::endl;;
}

std::string request::Handler::getCGIPath()
{
  return getHeader(" cgipath");
}

std::string request::Handler::getCGIExtension()
{
  return getHeader(" cgi");
}

bool request::Handler::isCGI(std::vector<std::string> *cgipath, std::vector<std::string>* cgi, std::string extension, std::string resource, std::string &root)
{
  //std::cout << "Is CGI?" << std::endl;
  //std::cout << "CGI " << cgi << "CGIPath " << cgipath << resource.empty() << root.empty() << std::endl;
  if (!cgipath || !cgi || cgipath->empty() || cgi->empty() || resource.empty() || root.empty())
    return false;
  //std::cout << "Is CGI?" << std::endl;
  std::vector<std::string>::iterator it_path = cgipath->begin();
  std::vector<std::string>::iterator it_extension = cgi->begin();
  while (it_path != cgipath->end() && it_extension != cgi->end())
  {
 //   if (resource.find(*it_path) == root.size() && *it_extension == extension)
    //std::cout << extension << " " << *it_extension << std::endl;
    if (*it_extension == extension)
    {
      header_[" cgipath"] = *it_path;
      header_[" cgi"] = *it_extension;
      return true;
    }
    it_extension++;
    it_path++;
  }
  return false;
}

bool request::Handler::handleCGI(std::vector<std::string> *cgipath, std::vector<std::string> *cgi, std::string extension, std::string resource, int &statuscode, std::string &root)
{
  if (isCGI(cgipath, cgi, extension, resource, root))
  {
    if (access(resource.c_str(), X_OK) != -1)
      return true;
    statuscode = request::FORBIDDEN;// ERROR == Content-Type html
  }
  return false;
}

std::string request::Handler::replaceBegining(std::string &src, std::string query, std::string replace)
{
  if (query.empty() || replace.empty() || src.find(query) != 0)
    return ""; // Error.
  src.replace(0, query.size(), replace); // FYI just the first coincidence not more.
  return src;
}

void request::Handler::applyRootOrAlias(std::string &resource, std::string uri, std::string root)
{
  if (getHeader(" alias").empty()) // Not alias.
  {
    if (!root.empty() && root.size() > 0 && root.at(root.size() - 1) == '/' && !resource.empty() && resource.size() > 0 && resource.at(0) == '/')
      root.erase(root.size() - 1, 1);
    resource = root + resource;
    return ;
  }
  uri = getHeader(" alias");
  //if (uri.at(uri.size() - 1) != '/')
  //  uri += '/';
  //std::cout << "alias: " << uri << " resource: " << resource << " root: " << root << std::endl;
  //if (uri.size() - 1] != '/')
  replaceBegining(resource, uri, root);
}

std::vector<std::deque<char> > request::Handler::getMultiParts(std::deque<char> original, std::string delim)
{
  //TODO change \r\n with eol_
  eol_ = "\r\n";
  std::string delim_end = eol_ + "--" + delim + "--" + eol_;
  delim = eol_ + "--" + delim + eol_;
  header_[" boundary"] = delim;
  // This is for don't make a if for the first part.
  original.insert(original.begin(), eol_.begin(), eol_.end());
  std::deque<char>::iterator begin = original.begin();
  std::deque<char>::iterator end = request::Handler::findDeque(begin, original.end(), delim);
  if (end != begin || request::Handler::findDeque(begin, original.end(), delim_end) == original.end())
    return multipart_;
  while (end != original.end())
  {
    begin = end;
    end = request::Handler::findDeque(begin + delim.size(), original.end(), delim);
    if (end == begin + delim.size())
      return std::vector<std::deque<char> >();
    if (end == original.end())
    {
      end = request::Handler::findDeque(begin + delim.size(), original.end(), delim_end);
      if (std::distance(end, original.end()) != static_cast<ssize_t>(delim_end.size()))
        return std::vector<std::deque<char> >();
      multipart_.push_back(std::deque<char>(begin, end));
      return multipart_;
    }
    multipart_.push_back(std::deque<char>(begin, end));
  }
  return std::vector<std::deque<char> >();
}

bool request::Handler::isFilePart(std::deque<char> & part, int & status)
{
  status = false;
  if (part.empty())
    return false;
  if (!validPart(part))
    return false;
  std::string filename = getFilename(part);
  if (filename.empty())
    return false;
  filename = getValidFilename(getUploadStore() + filename);
  if (filename.empty())
    return false;
  // TODO This is should set in PreviousHeaders
  eol_ = "\r\n";
  std::deque<char>::iterator it = findDeque(part.begin(), part.end(), eol_);
  if (it != part.begin())
  {
    status = true;
    return status;
  }
  it = findDeque(it + eol_.size(), part.end(), eol_);
  if (it == part.end())
  {
    status = true;
    return status;
  }
  it = findDeque(it + eol_.size(), part.end(), eol_);
  if (it == part.end())
    status = true;
  it = findDeque(it + eol_.size(), part.end(), eol_);
  if (it == part.end())
    status = true;
  it = findDeque(it + eol_.size(), part.end(), eol_);
  if (it == part.end())
    status = true;
  return true;
}

bool request::Handler::writeFiles(std::deque<char> & part)
{
  std::deque<char>::iterator it = part.begin();
  it = findDeque(it, part.end(), eol_);
  it += eol_.size();
  it = findDeque(it + eol_.size(), part.end(), eol_);
  it += eol_.size();
  it = findDeque(it + eol_.size(), part.end(), eol_ + eol_);
  it += eol_.size() * 2; // Two following new lines
  std::string filename = getValidFilename(getUploadStore() + getFilename(part));
  if (filename.empty())
    return false;
  std::ofstream outfile(filename.c_str());
  if (!outfile.is_open())
    return false;
  while (it != part.end())
  {
    outfile << *it;
    if (!outfile.good())
      return false;
    it++;
  }
 // I don't want a new line
 // outfile.close();
  files_.push_back(filename);
  return true;
}


std::string request::Handler::getValidFilename(std::string filename)
{
  size_t counter = 0;
  if (filename.empty())
    return filename;
  if (access(filename.c_str(), F_OK) == -1)
    return filename;
  std::string filebk = filename;
  while (access(filename.c_str(), F_OK) != -1)
  {
    filename = filebk + to_string(counter);
    counter++;
  }
  return filename;
}

bool request::Handler::validPart(std::deque<char> & part)
{
  if (part.empty())
    return false;
  std::deque<char>::iterator it = findDeque(part.begin(), part.end(), "; filename=\"");
  if (it == part.end())
    return true;
  std::deque<char>::iterator end = findDeque(it, part.end(), "\"");
  if (end == (it + 13))
    return false;
  return true;
}

std::string request::Handler::getFilename(std::deque<char> & part)
{
  if (part.empty())
    return "";
  std::deque<char>::iterator it = findDeque(part.begin(), part.end(), "; filename=\"");
  if (it == part.end())
    return "";
  it += 12;
  std::deque<char>::iterator end = findDeque(it, part.end(), "\"");
  if (end == part.end() || it == end)
    return "";
  return decodeURI(std::string(it, end));
}

bool request::Handler::handleUploadStore()
{
  if (multipart_.empty())
    return false;
  std::vector<std::deque<char> >::iterator it = multipart_.begin();
  int status = false;
  size_t counter_files = 0;
  while (it != multipart_.end())
  {
    // This is check before    validPart(part);
    if (isFilePart(*it, status))
    {
      if (status)
      {
        std::cout << "BADREQUEST" << std::endl;
        statuscode_ = request::BADREQUEST;
        return false;
      }
      if (!writeFiles(*it))
      {
        std::cout << "INTERNALSERVERERROR" << std::endl;
        statuscode_ = request::INTERNALSERVERERROR;
        return false;
      }
      else
      {
        counter_files++;
        if (multipart_.size() == counter_files)
        {
          body_.clear();
          break ;
        }
        std::deque<char>::iterator it_v = findDeques(body_.begin(), body_.end(), (*it).begin(), (*it).end());
        if (it_v == body_.end())
        {
          statuscode_ = request::INTERNALSERVERERROR;
          return false;
        }
        body_.erase(it_v, it_v + (*it).size());
      }
    }
    it++;
  }
  statuscode_ = request::CREATED;
  if (getUploadPass().empty())
    body_.clear();
  return true;
}


bool request::Handler::handlerUpload(std::string &resource)
{
  // Quard errors begin
  // std::cout << "Enter HandlerUpload" << std::endl;
  // I'm overwriting the error of processRequest in case is method post.
  // std::cout << "statuscode_ = " << statuscode_ << std::endl;
  if (getHeader(" method") != "post")
  {
    std::cout << "Error invalid method " << std::endl;
    statuscode_ = request::METHODNOTALLOWED;
    return true;
  }
  else if (getHeader("content-type").find("multipart/form-data") != 0)
  {
    std::cout << "Error invalid Content-Type" << std::endl;
    statuscode_ = request::UNSUPPORTEDMEDIATYPE;
    return true;
  }
  else if (!getUploadPass().empty() && (!isValidPath(getUploadPass().c_str()) || access(getUploadPass().c_str(), W_OK | X_OK) == -1))
  {
    std::cout << "getUploadPass path doesn't exist or don't have write or exec permision" << std::endl;
    statuscode_ = request::INTERNALSERVERERROR;
    return true;
  }
  else if (!getUploadStore().empty() && (!isValidPath(getUploadStore().c_str()) || access(getUploadStore().c_str(), W_OK) == -1))
  {
    std::cout << "getUploadStore path doesn't exist or don't have write permision" << std::endl;
    statuscode_ = request::INTERNALSERVERERROR;
    return true;
  }
  // Quard errors end.
  if (!getUploadStore().empty())
  {
    //std::cout << "handleUploadStore" << std::endl;
    handleUploadStore();
  }
  if (!getUploadPass().empty() && (statuscode_ == request::CREATED || statuscode_ == request::OK))
  {
    //std::cout << "handleUploadPass" << std::endl;
    statuscode_ = request::OK;
    resource = getUploadPass();
  }
 // if (!getUploadPass().empty())
 // std::cout << "Leaving with out enter in one if" << std::endl;
  return true;
}

std::string request::Handler::handlerResource(std::vector<std::string> *index, std::string root, std::string &resource, std::vector<std::string> *cgi, int &statuscode, int request_type)
{
  // Debugging purpose
  //std::cout << "before replace begining: " << resource  << std::endl;
  //std::cout << "rsc: " << resource << std::endl;
  //std::cout << "uri: " << getHeader(" uri") << std::endl;
//  std::cout << "root: " << getHeader(" root") << std::endl;
  //std::cout << "statuscode: " << statuscode_ << std::endl;
 if (statuscode == request::OK && !getHeader(" returnstatus").empty())
  {
    //std::cout << "getheader returnstatus " <<  getheader(" returnstatus");
    statuscode_ = ::atoi(getHeader(" returnstatus").c_str());
    //std::cout << "el codigo de estado es: " << statuscode_ << std::endl;
    statuscode = statuscode_;
    if (!getHeader(" returnstring").empty())
    {
      customreturn_ = true;
      return "";
    }
  }
 if (getUploadPass().empty())
   applyRootOrAlias(resource, getHeader(" uri"), getHeader(" root"));
  // Debugging purpose.

  //std::cout << "after replace begining: " << resource  << std::endl;
  //std::cout << "el codigo de estado es: " << statuscode_ << std::endl;

  if (statuscode == request::OK && !isValidPath(resource.c_str()))
    statuscode = request::NOTFOUND;
  else if (statuscode_ == request::OK && !getVector(" cgi") && !getVector(" cgipath") && getHeader(" root").empty() && getHeader(" alias").empty()) // This is for no root or alias in server or location.
  {
    statuscode = request::NOTFOUND;
}

  //std::cout << "el codigo de estado es: " << statuscode_ << std::endl;

  if (statuscode == request::OK && isDirectory(resource.c_str()) && (resource.empty() || (!resource.empty() && resource.size() > 0 && resource.at(resource.size() - 1) != '/')))
    resource += '/';
  if (statuscode == request::OK && resource == "/")// For security concerns.
    resource = "/dev/null";
  // Debugging purpose.
  //std::cout << "after replace check path && add slash: " << resource << std::endl;
  // Change for add include headers errors
  resource_ = resource;
  
  if (statuscode == request::OK)
  {
    // Debug purpose.
    //if (targetLocation_)
    //  debug_Location(targetLocation_);
   // if (index)
    //    sprintVector(*index);
    std::string indexstr = getIndex(resource, index);
   //std::cout << "this is the index: " << indexstr << std::endl; // Until here all fine.
    if (resource.empty() && request_type != request::DELETE)
      resource = "/dev/null" + indexstr;
    else if (isDirectory(resource.c_str()) && request_type != request::DELETE)
      resource += indexstr;//////////
  //std::cout << "after add index begining: " << resource  << std::endl;
  // Change for add include headers errors
    resource_ = resource;
    if (access(resource.c_str(), F_OK) == -1)
    {
      std::cout << "Doesn't exist the file" << std::endl;
      statuscode = request::NOTFOUND;
    }
    else if (access(resource.c_str(), R_OK) == -1)
    {
      std::cout << "Doesn't have read permision to the file" << std::endl;
      statuscode = request::FORBIDDEN;
    }
    else if (request_type == request::DELETE && access(resource.c_str(), W_OK) == -1)
    {
      std::cout << "Doesn't have write permision to the file and i have to delete" << std::endl;
      statuscode = request::FORBIDDEN;
    }
  }
  // This should be another function.
  std::string extension = getExtension(resource);
  if (root.empty())
    root = "/dev/null";

  //std::cout << "root: " << root << std::endl;

  //std::cout << "extension: " << extension << std::endl;
  if (statuscode == request::OK && getVector(" cgipath") && handleCGI(getVector(" cgipath"), cgi, extension, resource, statuscode_, root))
    return "";
  std::string contentType = getContentType(extension);
  //std::cout << "ContentType: " << contentType << std::endl;
  if (statuscode == request::OK && isDirectory(resource.c_str()) && getHeader(" directoryListing").empty())
    statuscode = request::NOTFOUND;
  if (contentType.empty() && isDirectory(resource.c_str()))
    contentType = "text/html";
  else if (contentType.empty())
    contentType = "application/octet-stream";
  //std::cout << "Leaving" << std::endl;
  return contentType;
}

void request::Handler::handleDelete(std::string const & resource, int &statuscode)
{
  if (isDirectory(resource.c_str()))
  {
    statuscode = request::INTERNALSERVERERROR;
    std::cout << "Trying to delete a directory" << std::endl;
  }
  else if (isFile(resource.c_str()))// May I change this for the delete handle in some special way or not.
  {
    //std::cout << "Deleting coming soon" << std::endl;
    statuscode_ = request::NOCONTENT;
    if (!network::DeleteFile::deleteFile(resource))
      statuscode_ = request::INTERNALSERVERERROR;

  }
  else // on case that the isn't a regular file or a Directory, per example a socket socket ...
    statuscode_ = request::NOTACCEPTABLE;
}

bool request::Handler::handleCustomErrors(std::vector<std::string> *index, std::string root, std::string &resource, std::vector<std::string> *cgi, int & status, int request_type, std::deque<char> & content)
{
  int status2 = status;
  status2 = request::OK;
  std::string contentType = handlerResource(index, root, resource, cgi, status2, request_type);
  if (contentType.empty()) // CGI ?
    return true;
  //std::cout << "statuscode og : " << getDefaultError(statuscode_) << std::endl;
  //std::cout << "statuscode: " << status2 << std::endl;
  //std::cout << "Custom error path: " << resource << std::endl;
  if (status2 == request::OK && isFile(resource.c_str()))
  {
    content = getFile(resource);
    if (content.empty())
      statuscode_ = request::NOCONTENT;
    //std::cout << "content size: " << content.size() << std::endl;
  }
  else if (!(getDefaultError(statuscode_).empty()))
  {
    std::string defaultError = getDefaultError(statuscode_);
    content.insert(content.end(), defaultError.begin(), defaultError.end());
  }
  return false;
}

void request::Handler::handlerMethod(std::string &str)
{
  //std::cout << "Request type" << request_type_ << std::endl;
  str += "Allow: ";
  if (request_type_ == GET)
    str += "GET";
  else if (request_type_ == POST)
    str += "POST";
  else if (request_type_ == DELETE)
    str += "DELETE";
  if (request_type_ & GET)
    str += "GET, ";
  if (request_type_ & POST && !(request_type_ & DELETE))
    str += "POST";
  if (request_type_ & POST && request_type_ & DELETE)
    str += "POST, ";
  if (request_type_ & DELETE && request_type_ != DELETE)
    str += "DELETE";
  str += "\r\n";
}

std::deque<char> request::Handler::handleRedirection(std::string &str, std::deque<char> &response)
{
  std::string version;
  if (getHeader(" version").empty())
    version = "1.0";
  else
    version = getHeader(" version");
  std::string resource = getHeader(" resource");
  resource.replace(0, getHeader(" uri").size(), getHeader(" returnstring"));
  str += "Location: " + resource + "\r\n";
  return createHeaders(statuscode_,  version, getKeepalive(), "text/html", response.size(), str, response);
}

std::deque<char> request::Handler::customMsg(std::string &str, std::deque<char> &response)
{
  std::string version;
  if (getHeader(" version").empty())
    version = "1.0";
  else
    version = getHeader(" version");
  std::string custom = getHeader(" returnstring");
  response.insert(response.end(), custom.begin(), custom.end());
  createHeaders(statuscode_,  version, getKeepalive(), "text/html", response.size(), str,  response);
  return response;
}

void request::Handler::validStatus(std::string &resource, std::deque<char> &content, std::string &contentType)
{
  if (request_type_ == request::DELETE)
    handleDelete(resource, statuscode_);
  else if (isFile(resource.c_str()))
  {
    content = getFile(resource);
    if (content.empty())
      statuscode_ = request::NOCONTENT;
  }
  else if (isDirectory(resource.c_str()) && getHeader(" directoryListing").size())
  {
    content = getDir(resource);
    contentType = "text/html";
  }
}

bool request::Handler::invalidStatus(std::deque<char> &content, std::string &resource, std::string custom_error, std::string & contentType)
{
  resource = custom_error;
  contentType = "text/html";
  // std::cout << "statuscode_ = " << statuscode_ << std::endl;
  if (resource.empty()) // No custom error.
  {
    std::string defaulterror = getDefaultError(statuscode_);
    if (!defaulterror.empty())
      content.insert(content.end(), defaulterror.begin(), defaulterror.end());
  }
  else if (handleCustomErrors(getVector(" index"), getHeader(" root"), resource, getVector(" cgi"), statuscode_, request_type_, content))
    return true;
  return false;
}

bool request::Handler::includeHeader(std::deque<char> & response, bool  status, std::string customstr, std::string contentType)
{
  if (status)
  {
    response = createHeaders((status ? request::OK: request::INTERNALSERVERERROR), getHeader(" version"), getKeepalive(), contentType, response.size(), customstr, response);
    return status;
  }
  std::map<int, std::string> custom_errors;
  setCustomErrors(custom_errors);
  statuscode_ = request::INTERNALSERVERERROR;
  if (invalidStatus(response, resource_, custom_errors[statuscode_], contentType))
  {
    files_.insert(files_.begin(), resource_.c_str());
    cgi::CgiManager cgi(files_, body_, header_);
    response = cgi.generateResponse(*this);//
    return status;
  }
  std::string custom;
  if (statuscode_== METHODNOTALLOWED)
    handlerMethod(custom);
  createHeaders(statuscode_,  getHeader(" version"), getKeepalive(), contentType, response.size(), custom, response);
  return status;
}

std::string request::Handler::debug_Cookie()
{
  std::string str;
  if (getVector("cookie"))
  {
    sprintVector(*getVector("cookie"));
    if (isStringInsideVector("cookie", "custom=bla"))
      std::cout << "Cookie is Set!" << std::endl;
    else
      str = "Set-Cookie: custom=bla\r\n";
  }
  return str;
}

std::deque<char> request::Handler::createHeaders(int statuscode, std::string version, bool keepalive, std::string contenttype, size_t contentsize, std::string customstr, std::deque<char> &headers)
{
  if (version.empty() && statuscode != request::NOTIMPLEMENTED)
    return headers;
  if (version.empty())
    version = "1.0";
  std::string str;
  str = "HTTP/" + version + " " + to_string(statuscode) + " " + getStatusMSG(statuscode) + "\r\n";
  str += "Server: webserv\r\n";
  str += "Connection: " + std::string(keepalive ? "keep-alive": "close") + "\r\n";
  str += "Date: " + request::Handler::getDate() + "\r\n";
  if (!contenttype.empty())
    str += "Content-Type: " + contenttype + "\r\n";
  str += "Content-Length: " + to_string(contentsize) + "\r\n";
  str += customstr;
  str += "\r\n";
  headers.insert(headers.begin(), str.begin(), str.end());
  return headers;
}

std::string create_error_page(int error_code, std::string error_msg)
{
    std::stringstream ss;
    std::string website;
    ss << error_code;
    std::string	final_error_code = ss.str();
    if (error_code == 201)
      website = "<!DOCTYPE html> <html lang=\"en\"> <head> <meta charset=\"UTF-8\"> <title>" + error_msg + "</title> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"> <link rel='stylesheet' href='https://fonts.googleapis.com/css?family=Lato:100,300'> <style> * { transition: all 0.6s; } html { height: 100%; } body { font-family: 'Lato', sans-serif; color: #888; margin: 0; } #main { display: table; width: 100%; height: 100vh; text-align: center; } .fof { display: table-cell; vertical-align: middle; } .fof h1 { font-size: 50px; display: inline-block; padding-right: 12px; animation: type .5s alternate infinite; } @keyframes type { from { box-shadow: inset -3px 0px 0px #888; } to { box-shadow: inset -3px 0px 0px transparent; } } </style> </head> <body> <div id=\"main\"> <div class=\"fof\"> <h1>Success " + final_error_code + "</h1> <h2>" + error_msg + "</h2> </div> </div> </body> </html>\r\n";
    else
      website = "<!DOCTYPE html> <html lang=\"en\"> <head> <meta charset=\"UTF-8\"> <title>" + error_msg + "</title> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"> <link rel='stylesheet' href='https://fonts.googleapis.com/css?family=Lato:100,300'> <style> * { transition: all 0.6s; } html { height: 100%; } body { font-family: 'Lato', sans-serif; color: #888; margin: 0; } #main { display: table; width: 100%; height: 100vh; text-align: center; } .fof { display: table-cell; vertical-align: middle; } .fof h1 { font-size: 50px; display: inline-block; padding-right: 12px; animation: type .5s alternate infinite; } @keyframes type { from { box-shadow: inset -3px 0px 0px #888; } to { box-shadow: inset -3px 0px 0px transparent; } } </style> </head> <body> <div id=\"main\"> <div class=\"fof\"> <h1>Error " + final_error_code + "</h1> <h2>" + error_msg + "</h2> </div> </div> </body> </html>\r\n";
    return (website);
}

std::string request::Handler::getDefaultError(int key)
{
  static std::map<int, std::string> default_errors;
  if (default_errors.empty())
  {
    default_errors[CREATED] 				= create_error_page(CREATED, "Created");
    default_errors[INTERNALSERVERERROR]		= create_error_page(INTERNALSERVERERROR, "INTERNAL SERVER ERROR");
    default_errors[NOTFOUND] 				= create_error_page(NOTFOUND, "NOT FOUND");
    default_errors[NOTIMPLEMENTED] 				= create_error_page(NOTIMPLEMENTED, "NOTIMPLEMENTED");
    default_errors[FORBIDDEN] 				= create_error_page(FORBIDDEN, "FORBIDDEN");
    default_errors[BADREQUEST] 				= create_error_page(BADREQUEST, "BAD REQUEST");
    default_errors[METHODNOTALLOWED] 		= create_error_page(METHODNOTALLOWED, "METHOD NOT ALLOWED");
    default_errors[HTTPVERSIONNOTSUPPORTED] = create_error_page(HTTPVERSIONNOTSUPPORTED, "HTTP VERSION NOT SUPPORTED");
    default_errors[NOTACCEPTABLE] 			= create_error_page(NOTACCEPTABLE, "NOT ACCEPTABLE");
  }
  return default_errors[key];
}

void request::Handler::setAllowedMethod(int &allowedMethod)
{
  std::vector<std::string> acceptedMethods;
  if (targetLocation_)
    acceptedMethods = targetLocation_->getAcceptMethod();
  if (acceptedMethods.empty())
  {
    allowedMethod = request::GET | request::POST;
    return ;
  }
  std::vector<std::string>::iterator it = acceptedMethods.begin();
  std::vector<std::string>::iterator end = acceptedMethods.end();
  allowedMethod = 0;
  while (it != end)
  {
    if (*it == "GET")
      allowedMethod |= request::GET;
    else if (*it == "POST")
      allowedMethod |= request::POST;
    else if (*it == "DELETE")
      allowedMethod |= request::DELETE;
    else
    {
      allowedMethod = request::RT_ERROR;
      return ;
    }
    it++;
  }
}

void request::Handler::setAllowedMethod(std::string &allowedMethod)
{
  std::vector<std::string> acceptedMethods;
  if (targetLocation_)
    acceptedMethods = targetLocation_->getAcceptMethod();
  allowedMethod = "Allow: ";
  if (acceptedMethods.empty())
  {
    allowedMethod += "GET, POST\r\n";
    return ;
  }
  size_t index = 0;
  while (index < acceptedMethods.size())
  {
    if (acceptedMethods[index] == "GET" && getUploadPass().empty() && getUploadStore().empty())
    {
      allowedMethod += "GET";
      if (index + 1 != acceptedMethods.size())
        allowedMethod += ", ";
    }
    else if (acceptedMethods[index] == "POST")
    {
      allowedMethod += "POST";
      if (index + 1 != acceptedMethods.size() && getUploadStore().empty() && getUploadPass().empty())
        allowedMethod += ", ";
    }
    else if (acceptedMethods[index] == "DELETE" && getUploadPass().empty() && getUploadStore().empty())
    {
      allowedMethod += "DELETE";
      if (index + 1 != acceptedMethods.size())
        allowedMethod += ", ";
    }
    index++;
  }
  allowedMethod += "\r\n";
}

std::deque<char> request::Handler::getResponse(void)
{
//  std::cout << std::string(request_.begin(), request_.end()) << std::endl;
  std::deque<char> response = responseRequest();
  //std::cout << std::string(body_.begin(), body_.end()) << std::endl;
  //this->reset();
  return response;
}


std::deque<char> request::Handler::responseRequest(void)
{
  std::map<int, std::string> custom_errors;
  int allowedMethod;
  //  debug_Cookie();
  // settings vars.
  initVars(custom_errors);
  setAllowedMethod(allowedMethod);
  std::string resource = getHeader(" resource");
  std::string str;
  std::deque<char> response;
  std::string contentType;
  //std::cout << "statuscode: " << statuscode_ << std::endl;
  if (request_type_ != request::RT_ERROR && !(request_type_ & allowedMethod))
    statuscode_ = METHODNOTALLOWED;
  //std::cout << "statuscode: " << statuscode_ << std::endl;
  if (statuscode_ == request::OK && (!getUploadStore().empty() || !getUploadPass().empty()))
  {
    if (handlerUpload(resource) &&  statuscode_ == request::CREATED && getUploadPass().empty())
    {
      std::string defaulterror = getDefaultError(statuscode_);
      if (!defaulterror.empty())
        response.insert(response.end(), defaulterror.begin(), defaulterror.end());
      createHeaders(statuscode_,  getHeader(" version"), getKeepalive(), "text/html", response.size(), "", response);
      return response;
    }
  }
  //std::cout << "statuscode: " << statuscode_ << std::endl;
  contentType = handlerResource(getVector(" index"), getHeader(" root"), resource, getVector(" cgi"), statuscode_, request_type_);
  //std::cout << "statuscode: " << statuscode_ << std::endl;
  if (contentType.empty() && isStatuscodeRedirect(statuscode_))
    return handleRedirection(str, response);
  else if (contentType.empty() && statuscode_ == request::OK) // CGI ?
  {
    files_.insert(files_.begin(), resource.c_str());
    cgi::CgiManager cgi(files_, body_, header_);
    return cgi.generateResponse(*this);//
  }
  else if (customreturn_)
    return customMsg(str, response);
  if (statuscode_ == request::OK)
    validStatus(resource, response, contentType);
  if (statuscode_ != request::OK && invalidStatus(response, resource, custom_errors[statuscode_], contentType))
  {
    files_.insert(files_.begin(), resource.c_str());
    cgi::CgiManager cgi(files_, body_, header_);
    return cgi.generateResponse(*this);//
  }
  std::string custom;
  if (statuscode_ == METHODNOTALLOWED)
    setAllowedMethod(custom);
  createHeaders(statuscode_,  getHeader(" version"), getKeepalive(), contentType, response.size(), custom, response);
  return response;
}
