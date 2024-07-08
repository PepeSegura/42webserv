// Copyright 2024 42Madrid GPL
// Author: alvjimen
#include "Request.hpp"

bool request::Handler::debug_info(bool value, std::string OK, std::string KO)
{
  std::string print = value ? OK : KO;
  if (print.empty())
    return value;
  std::cout << print << std::endl;
  return value;
}

// This func exist c11 xD;

void request::Handler::setReturn()
{
  if (!targetLocation_)
    return ;
  std::pair< size_t, std::string> status_string = targetLocation_->getReturn();
  if (status_string.first != 0)
    header_[" returnstatus"] = to_string(status_string.first);
  if (!status_string.second.empty())
    header_[" returnstring"] = status_string.second;
}

void request::Handler::setCustomErrors(std::map<int, std::string> &custom_errors)
{
  if (!targetserver_)
    return ;
  std::vector<std::pair<std::string, std::string> > vector = targetserver_->getErrorPage();
  std::vector<std::pair<std::string, std::string> >::iterator it = vector.begin();
  std::vector<std::pair<std::string, std::string> >::iterator end = vector.end();
  while (it != end)
  {
    custom_errors[::atoi((*it).first.c_str())] = (*it).second;
    it++;
  }
}

void request::Handler::initVars(std::map<int, std::string> &custom_errors)
{
  if (!targetserver_)
  {
    std::cout << "this shouldn't pass" << std::endl;
    std::cout << "I should set the targetServer && Location" << std::endl;
    setTargetServer("");
    if (targetserver_)
      setTargetLocation(getHeader(" resource"));
    else
      return ;
    //std::cout << "I should set the targetServer && Location" << std::endl;
  }
  setCustomErrors(custom_errors);
  header_[" root"] = getDocumentRoot();
  setReturn();
  setCGI();
  if (targetLocation_ && !targetLocation_->getIndex().empty())
    vector_[" index"] = targetLocation_->getIndex();
//  else
//    vector_[" index"] = targetserver_->getIndex();
  header_[" uri"] = getDocumentURI();
  directoryListing();
  if (!targetLocation_)
    return;
  upload_pass_ = targetLocation_->getUploadPass();
  upload_store_ = targetLocation_->getUploadStore();
  if (!upload_store_.empty() && upload_store_[upload_store_.size() - 1] != '/')
    upload_store_ += '/';
}

size_t request::Handler::getBMS()
{
  if (!targetserver_ && !targetLocation_)
    return 0;
  // this could a ternary operator
  if (!targetLocation_)
    return targetserver_->getCMBS();
  return (targetLocation_->getCMBS());
}

void request::Handler::setCGI()
{
  std::vector<std::pair<std::string, std::string> > cgi;
  if (targetLocation_)
    cgi = targetLocation_->getCgiPass();
  if (cgi.empty())
    cgi = targetserver_->getCgiPass();
  std::vector<std::pair<std::string, std::string> >::iterator it = cgi.begin();
  std::vector<std::pair<std::string, std::string> >::iterator end = cgi.end();
  while (it != end)
  {
    vector_[" cgipath"].push_back((*it).second);
    vector_[" cgi"].push_back((*it).first);
    it++;
  }
}

void request::Handler::directoryListing()
{
  if (targetLocation_ && targetLocation_->getAutoindex())
    header_[" directoryListing"] = "a"; // FYI I just check for is empty or not.
}

std::string request::Handler::getDocumentRoot()
{
  std::string root;
  if (targetLocation_ && targetLocation_->getAlias().empty())
    root = targetLocation_->getRoot();
  else if (root.empty())
  {
    if (targetLocation_)
      root = targetLocation_->getAlias();
    if (root.empty())
      root = targetserver_->getRoot();
    else
      header_[" alias"] = targetLocation_->getURI();
  }
//  std::cout << "root: " << root << std::endl;
  return root;
}

std::string request::Handler::getDocumentURI()
{
  std::string uri;
  if (targetLocation_)
    uri = targetLocation_->getURI();
  if (uri.empty())
    uri = "/";
  return uri;
}

std::string request::Handler::getContentType(std::string extension)
{
  static std::map<std::string, std::string> contentType;
  if (contentType.empty())
  {
    contentType["aac"] = "audio/aac";
    contentType["abw"] = "application/x-abiword";
    contentType["apng"] = "image/apng";
    contentType["arc"] = "application/x-freearc";
    contentType["avif"] = "image/avif";
    contentType["avi"] = "video/x-msvideo";
    contentType["azw"] = "application/vnd.amazon.ebook";
    contentType["bin"] = "application/octet-stream";
    contentType["bmp"] = "image/bmp";
    contentType["bz"] = "application/x-bzip";
    contentType["bz2"] = "application/x-bzip2";
    contentType["cda"] = "application/x-cdf";
    contentType["csh"] = "application/x-csh";
    contentType["css"] = "text/css";
    contentType["csv"] = "text/csv";
    contentType["doc"] = "application/msword";
    contentType["docx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
    contentType["eot"] = "application/vnd.ms-fontobject";
    contentType["epub"] = "application/epub+zip";
    contentType["gz"] = "application/gzip";
    contentType["gif"] = "image/gif";
    contentType["htm"] = "text/html";
    contentType["html"] = "text/html";
    contentType["ico"] = "image/vnd.microsoft.icon";
    contentType["ics"] = "text/calendar";
    contentType["jar"] = "application/java-archive";
    contentType["jpeg"] = "image/jpeg";
    contentType["jpg"] = "image/jpeg";
    contentType["js"] = "text/javascript";
    contentType["json"] = "application/json";
    contentType["jsonld"] = "application/ld+json";
    contentType["mid"] = "audio/midi";
    contentType["midi"] = "audio/midi";
    contentType["mjs"] = "text/javascript";
    contentType["mp3"] = "audio/mpeg";
    contentType["mp4"] = "video/mp4";
    contentType["mpeg"] = "video/mpeg";
    contentType["mpkg"] = "application/vnd.apple.installer+xml";
    contentType["odp"] = "application/vnd.oasis.opendocument.presentation";
    contentType["ods"] = "application/vnd.oasis.opendocument.spreadsheet";
    contentType["odt"] = "application/vnd.oasis.opendocument.text";
    contentType["oga"] = "audio/ogg";
    contentType["ogv"] = "video/ogg";
    contentType["ogx"] = "application/ogg";
    contentType["opus"] = "audio/opus";
    contentType["otf"] = "font/otf";
    contentType["png"] = "image/png";
    contentType["pdf"] = "application/pdf";
    contentType["php"] = "application/x-httpd-php";
    contentType["ppt"] = "application/vnd.ms-powerpoint";
    contentType["pptx"] = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
    contentType["rar"] = "application/vnd.rar";
    contentType["rtf"] = "application/rtf";
    contentType["sh"] = "application/x-sh";
    contentType["svg"] = "image/svg+xml";
    contentType["tar"] = "application/x-tar";
    contentType["tif"] = "image/tiff";
    contentType["tiff"] = "image/tiff";
    contentType["ts"] = "video/mp2t";
    contentType["ttf"] = "font/ttf";
    contentType["txt"] = "text/plain";
    contentType["vsd"] = "application/vnd.visio";
    contentType["wav"] = "audio/wav";
    contentType["weba"] = "audio/webm";
    contentType["webm"] = "video/webm";
    contentType["webp"] = "image/webp";
    contentType["woff"] = "font/woff";
    contentType["woff2"] = "font/woff2";
    contentType["xhtml"] = "application/xhtml+xml";
    contentType["xls"] = "application/vnd.ms-excel";
    contentType["xlsx"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
    contentType["xml"] = "application/xml";
    contentType["xul"] = "application/vnd.mozilla.xul+xml";
    contentType["zip"] = "application/zip";
    contentType["3gp"] = "video/3gpp; audio/3gpp";
    contentType["3g2"] = "video/3gpp2; audio/3gpp2";
    contentType["7z"] = "application/x-7z-compressed";
  }
  return contentType[extension];
}

std::string request::Handler::getStatusMSG(int statuscode)
{
  static std::map<int, std::string> statusCodeMap;
  if (statusCodeMap.empty())
  {
    statusCodeMap.insert(std::make_pair(request::CONTINUE, "CONTINUE"));
    statusCodeMap.insert(std::make_pair(request::OK, "OK"));
    statusCodeMap.insert(std::make_pair(request::CREATED, "CREATED"));
    statusCodeMap.insert(std::make_pair(request::NOCONTENT, "NOCONTENT"));
    statusCodeMap.insert(std::make_pair(request::BADREQUEST, "BADREQUEST"));
    statusCodeMap.insert(std::make_pair(request::NOTFOUND, "NOTFOUND"));
    statusCodeMap.insert(std::make_pair(request::FORBIDDEN, "FORBIDDEN"));
    statusCodeMap.insert(std::make_pair(request::UNSUPPORTEDMEDIATYPE, "UNSUPPORTEDMEDIATYPE"));
    statusCodeMap.insert(std::make_pair(request::NOTIMPLEMENTED, "NOTIMPLEMENTED"));
    statusCodeMap.insert(std::make_pair(request::INTERNALSERVERERROR, "INTERNALSERVERERROR"));
    statusCodeMap.insert(std::make_pair(request::METHODNOTALLOWED, "METHODNOTALLOWED"));
    statusCodeMap.insert(std::make_pair(request::CONTENTTOOLARGE, "CONTENTTOOLARGE"));
  }
  return statusCodeMap[statuscode];
}

std::string request::Handler::getDate(void)
{
  std::string date;
  std::time_t now = std::time(NULL);
  std::tm *gmtm = std::gmtime(&now);
  if (!gmtm)
    return ("Error");
  char buffer[50];
  std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", gmtm);
  date = buffer;
  return date;
}

std::string request::Handler::getExtension(std::string resource)
{
  size_t lastslash = resource.rfind('/');
  size_t lastdot = resource.rfind('.');
  std::string extension;
  if (lastdot <= lastslash || lastdot == std::string::npos)
    return "";
  return resource.substr(++lastdot);
}

bool request::Handler::isFile(const char *path)
{
  struct stat info;
  if(stat(path, &info) != 0) {
    return false;
  }
  return S_ISREG(info.st_mode);
}

bool request::Handler::isValidPath(const char* path)
{
  struct stat info;
  return (stat(path, &info) == 0);
}

bool request::Handler::isDirectory(const char* path) {
  struct stat info;
  if(stat(path, &info) != 0)
    return false;
  return S_ISDIR(info.st_mode);
}

std::string request::Handler::getIndex(std::string resource, std::vector<std::string> *index)
{
  if (!index || index->empty())
    return "";
  std::string resourcebk(resource);
  //std::cout << " going to the loop index size " << index->size() << std::endl;
  std::vector<std::string>::iterator it = index->begin();
  while (it != index->end())
  {
    resource = resourcebk + *it;
    if (!(*it).empty() && access(resource.c_str(), F_OK) != -1) // ERROR == Content-Type html
      return *it;
    it++;
  }
  return "";
}

std::deque<char> request::Handler::getFile(std::string filename)
{
  std::ifstream file(filename.c_str(), std::ios::binary);
  if (!file.is_open() || !file.good())
  {
    std::cerr << "Failed to open file." << std::endl;
    file.close();
    return std::deque<char> ();
  }
  char ch;
  std::deque<char> buffer;
  while (file.get(ch))
    buffer.push_back(ch);
  file.close();
  if (!file.eof())
  {
    std::cerr << "failed to read the file." << std::endl;
    return std::deque<char> ();
  }
  return buffer;
}

std::deque<char> request::Handler::readDirectoryEntries(DIR *dir, std::string const path)
{
  std::deque<char> files;
  struct dirent *entry;
  std::string filename;

  while ((entry = readdir(dir)) != NULL)
  {
    filename = entry->d_name;
    if (!path.empty() && path.size() > 0 && path.at(path.size() - 1) == '/')
      filename = "<a href=\"./" + filename+"\" >" + filename + "</a></br>\r\n";
    //filename = "<a href=\"" + path + "" + filename+"\" >" + filename + "</a></br>\r\n";
    else
      filename = "<a href=\"./" + filename+"\" >" + filename + "</a></br>\r\n";
    //filename = "<a href=\"" + path + "/" + filename+"\" >" + filename + "</a></br>\r\n";
    files.insert(files.end(), filename.begin(), filename.end());
  }
  return files;
}

std::deque<char> request::Handler::getDir(const std::string &path)
{
  DIR *dir = opendir(path.c_str());
  if (!dir)
    return std::deque<char> (); // Return an empty vector if the directory cannot be opened
  std::deque<char> files = readDirectoryEntries(dir, path);
  closedir(dir);
  return files;
}

bool request::Handler::isStatuscodeRedirect(int statuscode)
{
  return (statuscode >= request::MULTIPLECHOICES && statuscode <= request::PERMANENTREDIRECT);
}
