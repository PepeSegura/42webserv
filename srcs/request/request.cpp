// Copyright 2024 42Madrid GPL
// Author: alvjimen
#include <vector>
#include <cstring>
#include <sstream>
#include <cstdlib>
#include <algorithm>
#include "Request.hpp"

bool request::Handler::getKeepalive()
{
  return getHeader(" version") == "1.1" && getHeader("connection") != "close";
}

void request::Handler::reset()
{
  //keepalive_ = getkeepalive();
  body_.clear();
  vector_.clear();
  header_.clear();
  files_.clear();
  multipart_.clear();
  request_.clear();
  line_start_ = request_.begin();
  resource_ = "";
  request_status_ = 0;
  request_type_ = 0;
  statuscode_ = request::OK;
  targetLocation_ = NULL;
  targetserver_ = NULL;
  upload_pass_ = "";
  upload_store_ = "";
}

std::string request::Handler::getUploadStore() const
{
  return upload_store_;
}

std::string request::Handler::getUploadPass() const
{
  return upload_pass_;
}

void request::Handler::sprintVector(std::vector<std::string> const &vector)
{
  std::vector<std::string>::const_iterator it = vector.begin();
  while (it != vector.end())
  {
    std::cout << *it << std::endl;
    it++;
  }
}

std::ostream&	operator<<(std::ostream& COUT, request::Handler& han)
{
  if (han.getTargetserver())
    COUT << "is set targetserver_" << std::endl;
  if (han.getTargetLocation())
    COUT << "is set targetLocation_" << std::endl;
  han.printVector(COUT);
  han.printHeader(COUT);
  return COUT;
}

const ServerConfig *request::Handler::getTargetserver()
{
  return targetserver_;
}

const parser::Location *request::Handler::getTargetLocation()
{
  return targetLocation_;
}

std::ostream& request::Handler::printVector(std::ostream& COUT)
{
  std::map<std::string, std::vector<std::string> >::iterator it = vector_.begin();
  std::vector<std::string>::iterator strings_it;
  while (it != vector_.end())
  {
    COUT << (*it).first << ": ";
    strings_it = (*it).second.begin();
    while (strings_it != (*it).second.end())
    {
      COUT << " " << *strings_it;
      ++strings_it;
    }
    ++it;
  }
  COUT << std::endl;
  return COUT;
}

std::ostream& request::Handler::printHeader(std::ostream& COUT)
{
  std::map<std::string, std::string>::iterator it = header_.begin();
  while (it != header_.end())
  {
    COUT << (*it).first << ": " << (*it).second << std::endl;
    ++it;
  }
  return COUT;
}

request::Handler::Handler(std::vector<const ServerConfig*> servers) : upload_store_(""), upload_pass_(""),  servers_(servers),
  targetserver_(NULL), targetLocation_(NULL), request_status_(0), request_type_(0),
  statuscode_(200), customreturn_(0), line_start_(request_.begin())
{
}

request::Handler::Handler():  targetserver_(NULL), targetLocation_(NULL),
  request_status_(0), request_type_(0), statuscode_(200), customreturn_(0), line_start_(request_.begin())
{

}

request::Handler::~Handler()
{
}

// Token validity
bool request::Handler::isSp(char sp)
{
  return (sp == ' ' || sp == '\t');
}

bool request::Handler::isVchar(char vchar)
{
  return (vchar >= 0x21 && vchar <= 0x7E);
}

bool request::Handler::isObs_Text(char tchar)
{
  return (tchar < 0);
}

bool request::Handler::isTchar(char tchar)
{
  return (::isalnum(tchar) || validChar("!#$%&'*+-.^_`|~", tchar));
}

bool request::Handler::isField_vchar(char vchar)
{
  return (isVchar(vchar) || isObs_Text(vchar));
}

bool request::Handler::isField_content(char content)
{
  return (isField_vchar(content) || isSp(content));
}

bool request::Handler::isToken(std::string token) // token == Field name // Host
{
  std::string::iterator it = token.begin();
  if (token.empty())
    return false;
  while (it != token.end())
  {
    if (!isTchar(*it))
    {
      std::cout << "in this string " << token << " this is the first invalid char [" << *it << "]" << std::endl;
      return false;
    }
    it++;
  }
  return true;
}

bool request::Handler::isToken_v(std::vector<char>::iterator begin, std::vector<char>::iterator end)
{
  if (begin == end)
  {
    statuscode_ = BADREQUEST;
    return false;
  }
  while (begin != end)
  {
    if (!isTchar(*begin))
    {
      statuscode_ = BADREQUEST;
      return false;
    }
    begin++;
  }
  return true;
}

bool request::Handler::isField_value_v(std::vector<char>::iterator begin, std::vector<char>::iterator end)
{
  bool result = false;
  while (begin != end)
  {
    if (!isField_content(*begin))
    {
      std::cout << "bad Field content  ["  << *begin << "]" << std::endl;
      statuscode_ = BADREQUEST;
      return false;
    }
    if (!isSp(*begin))
      result = true;
    begin++;
  }
  return result;
}

bool request::Handler::isRequestLine_v(std::vector<char>::iterator begin, std::vector<char>::iterator end)
{
  std::vector<char>::iterator tkn;
  tkn = getNextToken(begin, end,  " \t");
  if (begin == end || !isMethod_v(begin, tkn))
    return false;
  header_[" method"] = toLower(std::string(begin, tkn));
  begin = tkn;
  tkn = getNextToken(begin, end, " \t");
  if (!isResource_v(begin, tkn))
  {
    request_type_ = RT_ERROR;
    if (statuscode_ == request::OK)
      statuscode_ = BADREQUEST;
    std::cout << "statuscode: " << statuscode_ << std::endl;
    return false;
  }
  if (tkn == end) // Get / ...
  {
    setTargetServer("");
    request_status_ = RS_END;
    return true;
  }
/*
  if (!getHeader(" host").empty() && setTargetServer(getHeader(" host")) && setTargetLocation(getHeader(" resource")))
  {

    std::cout << getHeader(" host") << std::endl;
  }
*/
  begin = tkn;
  tkn = getNextToken(begin, end, getEndOfLine_v(begin, end));
  if (!isHttpVersion_v(begin, tkn))
  {
    //std::cout << "ERROR" << std::endl;
    statuscode_ = BADREQUEST;
    request_type_ = RT_ERROR;
    request_status_ = RS_END;
  }
  else
    request_status_ = RS_FIRSTLINE;
  return true;
}

void request::Handler::addStringToVector(std::vector<std::string> &vector, std::string str)
{
  std::string s;
  std::stringstream ss(str);
  while (std::getline(ss, s, ','))
    vector.push_back(trim(s));
}

void request::Handler::addMultiHeader(std::string key, std::string value)
{
  value = trim(value);
  std::vector<std::string> *vector = getVector(key);
  if (!vector)
  {
    vector_[key] = std::vector<std::string>();
    vector = &vector_[key];
  }
  addStringToVector(*vector, value);
  if (getHeader(key).empty())
  {
    std::vector<std::string>::iterator it = vector->begin();
    std::string keystr;
    while (it != vector->end())
    {
      keystr += *it;
      it++;
    }
    header_[key] = keystr;
  }
  else
    header_[key] = getHeader(key) + value;
}

bool request::Handler::isHeaderField_v(std::vector<char>::iterator begin, std::vector<char>::iterator end)
{
  std::vector<char>::iterator tkn;
  tkn = getNextToken(begin, end, ":");
  if (!(begin == end || isToken_v(begin, tkn)))
    return false;
  //std::cout << "Hola" << std::endl;
  std::string key = toLower(std::string(begin, tkn));
  begin = tkn + 1;
  tkn = getNextToken(begin, end, "\r\n");
  if (!isField_value_v(begin, tkn))
    return false;
  //std::cout << "Hola" << std::endl;
  if (key == "host" && !isHost_v(begin, tkn))
      return false;
  else if (key == "transfer-encoding" || key == "cookie")
    addMultiHeader(key, trim(toLower(std::string(begin, tkn))));
  else if (key == "content-type")
  {
  //std::cout << "Hola" << std::endl;
    if (!isContentType(std::string(begin, tkn)))
      return false;
  }
  else
    header_[key] = trim(toLower(std::string(begin, tkn)));
  //std::cout << "Hola" << std::endl;
  return true;
}

std::vector<std::string> *request::Handler::getVector(std::string headerkey)
{
  std::map<std::string, std::vector<std::string> >::iterator it = vector_.find(headerkey);
  if (it == vector_.end())
    return NULL;
  return &(it->second);
}

std::string request::Handler::getHeader(std::string headerkey)
{
  std::map<std::string, std::string>::iterator it = this->header_.find(headerkey);
  if (it == header_.end())
    return "";
  return (*it).second;
}

void request::Handler::trailLines(std::vector<char>::iterator &line_begin, std::vector<char>::iterator end, std::string eol)
{
  while (line_begin != end && findVector(line_begin, end, eol) == line_begin)
    line_begin += eol.size();
}

/*
bool request::Handler::isEndHeader(std::vector<char> & request, std::vector<char>::iterator line_start, std::string eol)
{
  std::vector<char>::iterator line_begin = request.begin();
  trailLines(line_begin, request.end(), eol);
  if (line_begin == request.end())
    return false;
  std::vector<char>::iterator find = findVector(line_begin, request.end(), eol + eol);
  if (find == request.end())
    return false;

  return (std::distance(line_start, find)) == 0;
}
*/

bool request::Handler::isBeyondHeader(std::vector<char> & request, std::vector<char>::iterator line_start, std::string eol)
{
  std::vector<char>::iterator line_begin = request.begin();
  trailLines(line_begin, request.end(), eol);
  if (line_begin == request.end())
    return false;
  std::vector<char>::iterator find = findVector(line_begin, request.end(), eol + eol);
  if (find == request.end())
    return false;
  return (std::distance(line_start, find)) <= 0;
}

bool request::Handler::isStringInsideVector(std::string keyname, std::string search)
{
  std::vector<std::string> *key = getVector(keyname);
  return (key && std::find(key->begin(), key->end(), search) != key->end());
}

bool request::Handler::parseContentLenght(std::vector<char>::iterator begin, std::vector<char>::iterator end)
{
  std::string content = getHeader("content-length");
  if (content.empty() || begin == end)
    return true; // Error.
  std::istringstream iss(content);
  long int  length;
  if (!(iss >> length) || length < 0)
  {
    statuscode_ = BADREQUEST;
    request_type_ = RT_ERROR;
    return true;
  }
  std::istringstream iss2(getHeader(" bms"));
  long bodymaxsize;
  if (!(iss2 >> bodymaxsize) || bodymaxsize < 0)
  {
    std::cout << "BAD body size" << std::endl;
    statuscode_ = BADREQUEST;
    request_type_ = RT_ERROR;
    return true;
  }
  if (length > bodymaxsize)
  {
    std::cout << "length too large" << std::endl;
    statuscode_ = CONTENTTOOLARGE;
    request_type_ = RT_ERROR;
    return true;
  }
  if (length == 0) // if length == 0;
    return true;
  if (std::distance(begin, end) < length)
    return false; // Waiting for more data
  body_.clear();
  body_.insert(body_.begin(), begin, begin + length);
  request_status_ = RS_END;
  return true; // OK
}

bool request::Handler::parseChunkedBody(std::vector<char>::iterator &line_end, std::vector<char>::iterator end)
{
  body_.clear();
  std::string eol = getEndOfLine_v(line_start_, end);
  if (eol.empty() || line_end == end)
    return false;
  std::istringstream iss(std::string(line_start_, line_end));
  long int chunksize;
  if (!(iss >> std::hex >> chunksize))
  {
    statuscode_ = BADREQUEST;
    request_type_ = RT_ERROR;
    return true; // chunk not invalid.
  }
  long int bodymaxsize;
  std::istringstream iss2(getHeader(" bms"));
  if (!(iss2 >> bodymaxsize) || bodymaxsize < 0)
  {
    std::cout << "BAD body size" << std::endl;
    statuscode_ = BADREQUEST;
    request_type_ = RT_ERROR;
    return true;
  }
  if (chunksize == 0)
  {
    request_status_ = RS_END;
    return true;
  }
  if (static_cast<size_t>(bodymaxsize) < body_.size() + chunksize)
  {
    statuscode_ = CONTENTTOOLARGE;
    request_type_ = RT_ERROR;
    return true;
  }
  line_end += eol.size();
  if (std::distance(line_end, end) <= chunksize)
    return false;
  body_.insert(body_.end(), line_end, line_end + chunksize);
  line_end +=chunksize;
  if (line_end != findVector(line_end, end, eol))
  {
        statuscode_ = BADREQUEST;
    request_type_ = RT_ERROR;
    return true; // chunk not invalid.
  }
  return false;
}

bool request::Handler::setTargetServer(std::string host)
{
  if (servers_.empty())
    return false;
  std::vector<const ServerConfig *>::iterator it = servers_.begin();
  std::vector<const ServerConfig *>::iterator end = servers_.end();
  std::vector<const ServerConfig *>::iterator first = it;
  while (it != end)
  {
    if ((*it)->getServerName() == host)
    {
      targetserver_ = *it;
      return true;;
    }
    it++;
  }
  targetserver_ = *first;
  return true;;
}

bool request::Handler::setTargetLocation(std::string resource)
{
  if (!targetserver_)
    return false;
  locations_ = targetserver_->getLocations();
  std::vector<parser::Location> &locations = locations_;
  if (locations.empty())
    return false;
  std::vector<parser::Location>::iterator it = locations.begin();
  std::vector<parser::Location>::iterator end = locations.end();
  std::vector<parser::Location>::iterator it_bk = it;
  size_t length = 0;
  while (it != end)
  {
    if (resource.find((*it).getURI()) == 0)
    {
      if (length < (*it).getURI().size())
      {
  //      std::cout << "validURI " << (*it).getURI() << std::endl;
        length = (*it).getURI().size();
        it_bk = it;
      }
    }
    it++;
  }
  if (length)
    targetLocation_ = &(*it_bk);
  return true;
}

std::deque<char> const & request::Handler::getBody(void) const
{
  return body_;
}

bool request::Handler::isBody(std::vector<char> request)
{
  //std::cout << "Body" << std::endl;
  if (!targetserver_)
  	setTargetServer(getHeader(" header"));
  if (!targetLocation_)
	  setTargetLocation(getHeader(" resource"));
  header_[" bms"] = to_string(getBMS());
  body_.clear();
  if (request_status_ != RS_BODY)
    return true; // If RS_END  is ended if not an error has happend.
  std::string eol = getEndOfLine_v(request.begin(), request.end());
  if (eol.empty())
  {
    statuscode_ = BADREQUEST;
    request_type_ = RT_ERROR;
    return true; // chunk not invalid.
  }
  std::vector<char>::iterator line_end = request.begin();
  trailLines(line_end, request.end(), eol);
  if (line_end == request.end())
    return false;
  // End of Trailing begining eol
  line_end = findVector(line_end, request.end(), eol + eol);
  if (line_end == request.end())
    return false;
  line_end += eol.size() * 2;
  if (line_end == request.end())
    return false;
  bool  chunked = isStringInsideVector("transfer-encoding", "chunked");
  if (chunked && getHeader("content-length").size())
  {
    statuscode_ = BADREQUEST;
    request_type_ = RT_ERROR;
    return true;
  }
  if (!chunked)
    return parseContentLenght(line_end, request.end());
  line_start_ = line_end;
  while (containsVector(line_end, request.end(), eol))// if doesn't find a eol left the while
  {
    line_end = getNextToken(line_start_, request.end(), eol);
    if (line_start_ == request.end()) // End of lines
      break ;
    if (parseChunkedBody(line_end, request.end()))
      return true;
    line_start_ = line_end + eol.size();
  }
  return (false);
}

bool request::Handler::processRequest(char *buffer, size_t num_bytes)
{
  ssize_t distance = std::distance(request_.begin(), line_start_);
  distance = distance < 0 ? 0 : distance;
 // if (!distance)
//  {
//    std::cout << "Process Request " << std::endl;
    //this->reset();
//  }
  request_.insert(request_.end(), buffer, buffer + num_bytes);
  line_start_ = request_.begin() + distance;
  return this->processRequest(request_);
}

bool request::Handler::checkpart(std::deque<char> &part, std::string delim)
{
  std::deque<char>::iterator it;
  if (part.empty())
    return false;
  it = request::Handler::findDeque(part.begin(), part.end(), "Content-Disposition: form-data; name=\"");
  if (it == part.end() || std::distance(part.begin(), it) != static_cast<ssize_t>(delim.size()))
    return false;
  it += std::string("Content-Disposition: form-data; name:\"").size();
  if (it == request::Handler::findDeque(it, part.end(), "\""))
    return false;
  it = request::Handler::findDeque(it, part.end(), "\"; filename=\"");
  if (it == part.end())
    return true;
  it += std::string("\"; filename=\"").size();
  if (it != part.end() && it == request::Handler::findDeque(it, part.end(), "\""))
      return false;
  //std::cout << "Fine" << std::endl;
  return true;
}

bool request::Handler::checkMultipart(std::vector<std::deque<char> > multipart, std::string delim)
{
  if (multipart.empty())
    return false;
  std::vector<std::deque<char> >::iterator it = multipart.begin();
  while (it != multipart.end())
  {
    //std::cout << "Loop" << std::endl;
    if (!checkpart(*it, delim))
      return false;
    it++;
  }
  return true;
}

bool request::Handler::processRequest(std::vector<char> &request)
{
  if (request_status_ != RS_BODY && isHeader_v(request))
  {
//    std::cout << "Leaving processsRequest" << std::endl;
    return true;
  }
  if (request_status_ == RS_BODY && request_type_ != GET && isBody(request))
  {
    if (!getHeader(" boundary").empty())
    {
      //std::cout << "body_.size() " << body_.size() << std::endl;
      multipart_ = getMultiParts(body_, getHeader(" boundary"));
      if (multipart_.empty() || !checkMultipart(multipart_, getHeader(" boundary")))
      {
        request_status_ = BADREQUEST;
        request_type_ = RT_ERROR;
        statuscode_ = request::BADREQUEST;
        std::cout << "Error on multiparts" << std::endl;
      }
    }
    //std::cout << "Leaving processsRequest" << std::endl;
    return true;
  }
  return false;
}

/*
bool request::Handler::handleEmptyToken(std::vector<char>::iterator &line_end, const std::string &eol)
{
  if (line_start_ == line_end && eol.size())
  {
    std::cout << "empty Token" << std::endl;
    if (request_type_ == GET)
    {
      if (getHeader(" host").size())
        request_status_ = RS_END;
      else
      {
        statuscode_ = BADREQUEST;
        request_type_ = RT_ERROR;
      }
    } else // I should change this.
      request_status_ = RS_BODY;
    return true;
  }
  return false;
}
*/

bool request::Handler::processToken(std::vector<char>::iterator line_end)
{
  if (request_status_ == RS_NOEOL)
  {
    if (!isRequestLine_v(line_start_, line_end))
    {
      request_type_ = RT_ERROR;
      if (statuscode_ == request::OK)
        statuscode_ = BADREQUEST;
      std::cout << "isn'tRequestLine" << std::endl;;
      std::cout << std::string(line_start_, line_end) << std::endl;
    }
  }
  else if (!isHeaderField_v(line_start_, line_end))
  {
    std::cout << " HeaderField_v request error" << std::endl;
    std::cout << std::string(line_start_, line_end) << std::endl;
    statuscode_ = BADREQUEST;
    request_type_ = RT_ERROR; // Bad Request
  }
  return (request_type_ == RT_ERROR || request_status_ == RS_END);
}

bool request::Handler::isPreviousHeader(std::vector<char> &request,std::string &eol, std::vector<char>::iterator &line_end )
{
  eol = getEndOfLine_v(request.begin(), request.end());
  if (eol.empty())
    return (false);// Isn't Finish the first line
  line_end = line_start_;
  if (!containsVector(line_end, request.end(), eol))
    return false;
  trailLines(line_start_, request.end(), eol);
  return (line_start_ != request.end());
}

bool request::Handler::isHeader_v(std::vector<char> &request)
{
  std::string eol;
  std::vector<char>::iterator line_end;
  if (!isPreviousHeader(request, eol, line_end))
    return false;
  while (containsVector(line_end, request.end(), eol) && !isBeyondHeader(request, line_end, eol))
  {
    line_end = getNextToken(line_start_, request.end(), eol);
    if (line_end == request.end())
      break;
    // if (handleEmptyToken(line_end, eol))
    //   return request_status_ != RS_BODY;
    if (line_end == line_start_)
      break ;
    if (processToken(line_end))
      return true;
    if (line_end != request.end())
      line_end = line_end + eol.size();
    line_start_ = line_end;
  }
  if (isBeyondHeader(request, line_start_, eol))
  {
      if (getHeader("content-length").size() || isStringInsideVector("transfer-encoding", "chunked"))
      {
        //std::cout << "is body" << std::endl;
        if (getHeader(" method") != "get")
          request_status_ = RS_BODY;
        else
        {
          request_status_ = RS_END;
          request_type_ = RT_ERROR;
          statuscode_ = request::BADREQUEST;
        }
      }
      else
        request_status_ = RS_END;
  }
  return request_status_ == RS_END;
}

std::string const & request::Handler::getParamsString() const
{
  return header_.at(" parameters");
}

std::vector<std::string> const & request::Handler::getParamsSplit() const
{
  return vector_.at(" parameters");
}

