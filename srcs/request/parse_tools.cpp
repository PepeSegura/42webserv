#include <sys/stat.h>
#include "Request.hpp"

bool request::Handler::isDelim(std::string const delimiters, char cmp)
{
  return delimiters.find(cmp) != std::string::npos;
}

bool request::Handler::isBchar(char cmp)
{
  if (isalnum(cmp))
    return true;
  return (isDelim("'(),+_-./:=?", cmp));
}

bool request::Handler::isBoundary(std::string tkn)
{
  if (tkn.empty())
    return false;
  if (tkn.size() > 70)
    return false; // Too long boundary
  int space = false;
  std::string::iterator tkn_begin = tkn.begin();
  while (tkn_begin != tkn.end())
  {
    if (*tkn_begin == ' ')
      space = true;
    else if (isBchar(*tkn_begin))
      space = false;
    else
      return false;
    tkn_begin++;
  }
  if (!space) // Adding boundaries to env.
  {
  //   std::cout << tkn << std::endl;
    header_[" boundary"] = tkn;
  }
  return !space;
}

bool request::Handler::isContentTypeMultipart(std::string type)
{
  if (type.empty())
    return false;
  size_t start = type.find("multipart/");
  if (start != 0)
    return false;
  start = strlen("multipart/");
  if (start == type.find(";", start))
    return false;
  start = type.find(";", start);
  if (start == std::string::npos)
    return false;
  start = type.find("boundary=", start);
  if (start == std::string::npos)
    return false;
  return isBoundary(type.substr(start + strlen("boundary=")));
}

bool request::Handler::isParameter(std::string type)
{
  size_t semicolon = type.find(';');
  size_t equal = type.find('=');
  if (equal == std::string::npos)
    return false;
  if (semicolon != std::string::npos)
    semicolon++;
  while (type[semicolon] == ' ')
    semicolon++;
  if (!isToken(type.substr(semicolon, equal - (semicolon + 1))))
    return false;
  if (!isToken(trim(type.substr(equal + 1, std::string::npos))))
    return false;
  return true;
}

bool request::Handler::isContentType(std::string type)
{
//  std::cout << type << std::endl;
  size_t start = type.find("/");
  if (!start || std::string::npos == start)
    return false;
  if (type.find(" multipart/") == 0 && !isContentTypeMultipart(trim(type)))
    return false;
  if (!isToken(type.substr(1, start - 1)))
    return false;
  size_t end = start;
  if (type.find(";", end) == end) // FYI Avoid "multipart/;" or any case of following semicolons ";;"
    return false ;
  end = type.find(";", end);
  if (!isToken(type.substr(start + 1, end - (start + 1))))
    return false;
  start = end;
  end = type.find(";", start + 1);
  while (end != std::string::npos)
  {
    if (type.find(";", end) == end) // FYI Avoid "multipart/;" or any case of following semicolons ";;"
      return false ;
    if (std::string::npos == type.find(";", end))
      break ;
    if (!isParameter(toLower(trim(type.substr(start, end - start)))))
      return false;
    start = end;
    end = type.find(";", start + 1);
  }
  if (start != end && start != std::string::npos && !isParameter(type.substr(start, end - start)))
    return false;
  header_["content-type"] = trim(toLower(type));
  vector_["content-type"] = parseContentTypeParameters(header_["content-type"]);
  return true;
}

//bool isContentType

bool request::Handler::isSameLenght(std::vector<char>::iterator begin, std::vector<char>::iterator end, std::string const & search)
{
  return std::distance(search.begin(), search.end()) == std::distance(begin, end);
}

std::string request::Handler::trim(const std::string& str)
{
    std::string s(str);
    std::string::size_type start = s.find_first_not_of(' ');
    if (start != std::string::npos)
        s.erase(0, start);
    std::string::size_type end = s.find_last_not_of(' ');
    if (end != std::string::npos)
        s.erase(end + 1);
    return s;
}

std::string request::Handler::toLower(const std::string& str)
{
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

// Request Token
bool request::Handler::containsVector(std::vector<char>::iterator begin,
    std::vector<char>::iterator end, std::string search)
{
  return (std::search(begin, end, search.begin(), search.end()) != end);
}

std::deque<char>::iterator request::Handler::findDeques(std::deque<char>::iterator begin, std::deque<char>::iterator end, std::deque<char>::iterator sbegin, std::deque<char>::iterator send)
{
  if (begin > end)
    return end;
  return (std::search(begin, end, sbegin, send));
}

std::deque<char>::iterator request::Handler::findDeque( std::deque<char>::iterator begin, std::deque<char>::iterator end, std::string search)
{
  if (begin > end)
    return end;
  return (std::search(begin, end, search.begin(), search.end()));
}

std::vector<char>::iterator request::Handler::findVector(
    std::vector<char>::iterator begin, std::vector<char>::iterator end,
    std::string search)
{
  if (begin > end)
    return end;
  return (std::search(begin, end, search.begin(), search.end()));
}

bool request::Handler::equalVector(std::vector<char>::iterator begin,
    std::vector<char>::iterator end, std::string search)
{
  return (std::search(begin, end, search.begin(), search.end()) != end && isSameLenght(begin, end, search));
}

std::vector<char>::iterator request::Handler::getNextToken(
    std::vector<char>::iterator &begin, std::vector<char>::iterator end,
    std::string const delimiters) // For check errors if begin == end tkn_empty.
{
  if (begin == end)
    return end;
  while (begin != end && isDelim(delimiters, *begin))
    begin++;
  if (begin == end)
    return end;
  std::vector<char>::iterator token_end = begin; 
  while (token_end != end && !isDelim(delimiters, *token_end))
    token_end++;
  return token_end;
}

bool request::Handler::isMethod_v(std::vector<char>::iterator begin, std::vector<char>::iterator end)
{
  if (equalVector(begin, end, "GET"))
    this->request_type_ = GET;
  else if (equalVector(begin, end, "POST"))
    this->request_type_ = POST;
  else if (equalVector(begin, end, "DELETE"))
    this->request_type_ = DELETE;
  else
    this->statuscode_ = NOTIMPLEMENTED;
  return this->request_type_ != RT_ERROR;
}

std::string request::Handler::resolvePath(const std::string path)
{
    std::vector<std::string> components;
    size_t start = 0;
    if (path == "/")
      return std::string(path);
    while (path[start] == '/')
      start++;
    size_t end = path.find('/', start);
    while (end != std::string::npos)
    {
        std::string component = path.substr(start, end - start);
        if (component == "..") {
            // Go up one directory level
            if (!components.empty()) {
                components.pop_back();
            }
            else
              return ""; // ERROR ERROR 400 BAD REQUEST
        } else if (component != "." && !component.empty())
            components.push_back(component);
        start = end + 1;
        while (path[start] == '/')
          start++;
        end = path.find('/', start);
    }

    // Add remaining component
    std::string component = path.substr(start);
    if (!component.empty() && component != ".")
        components.push_back(component);
    else if ((component == "." || component.empty()) && components.empty())
      components.push_back(""); // FYI This is if is for edge case '/./' really funy xD
    // Construct resolved path
    std::string resolvedPath;
    for (size_t i = 0; i < components.size(); ++i) {
      resolvedPath += "/";
      resolvedPath += components[i];
    }
    return resolvedPath;
}


bool request::Handler::isHttpVersion_v(std::vector<char>::iterator begin,
std::vector<char>::iterator end)
{
  if (*begin != ' ')
    return false;
  begin++;
  std::vector<char>::iterator contain = findVector(begin, end, "HTTP/");
  if (begin != contain || contain == end)
    return false; 
  begin += 5;
  std::string version = std::string(begin, end);
  if (std::distance(begin, end) != 3 || !isdigit(*begin))
    return false;
  begin++;
  if (*begin != '.')
    return false;
  begin++;
  if (!isdigit(*begin))
    return false;
  if (version[0] > '1')
  {
    statuscode_ = HTTPVERSIONNOTSUPPORTED;
    request_type_ = RT_ERROR;
  }
  else if (version[0] == '1' && version[2] > '1')
  {
    statuscode_ = HTTPVERSIONNOTSUPPORTED;
    request_type_ = RT_ERROR;
  }
  header_[" version"] = version.c_str();
  return true;
}

std::vector<std::string> request::Handler::parseContentTypeParameters(std::string url)
{
    std::vector<std::string> parameters;
    std::stringstream ss(url);
    std::string param;
    while (std::getline(ss, param, ';'))
    {
      if (param.empty())
      {
        parameters.clear();
        break ;
      }
      parameters.push_back(std::string(param));
    }
    return parameters;
}

std::vector<std::string> request::Handler::parseURLParameters(std::string url)
{
    std::vector<std::string> parameters;
    std::stringstream ss(url);
    std::string param;
    while (std::getline(ss, param, '&'))
    {
      if (param.empty())
      {
        parameters.clear();
        break ;
      }
      parameters.push_back(std::string(param));
    }
    return parameters;
}

  // FYI a valid param is empty string xD ?#
bool request::Handler::isParams_v(std::vector<char>::iterator begin,
std::vector<char>::iterator end)
{
  if (begin ==  end)
    return false;
  std::vector<char>::iterator  start = findVector(begin, end, "//");
  if (start == end)
    start = begin;
  else
    start += 2;
  start = findVector(start,  end, "?");
  if (start == end)
    return false;
  std::vector<char>::iterator  tkn_end = findVector(++start, end, "#");
  header_["query_string"] = decodeURI(std::string(start, tkn_end));
  vector_[" parameters"] = parseURLParameters(header_[" parameters"]);
  return vector_[" parameters"].size();
}

// =============================================================================|
// TODO FYI My custom headers leading with a space ilegal char for a header ;)  |
// =============================================================================|

std::string replace(std::string str, std::string toReplace, std::string replace)
{
  size_t pos = 0;
  while ((pos = str.find(toReplace, pos)) != std::string::npos)
    str.replace(pos, toReplace.length(), replace);
  return str;
}

bool request::Handler::isResourcenotURI_v(std::vector<char>::iterator begin, std::vector<char>::iterator end)
{
  if (begin == end)
    return false;
  std::vector<char>::iterator start;
  std::vector<char>::iterator leadingthreefollowingslashes;
  start = begin;
  leadingthreefollowingslashes = start;
  start = findVector(start, end, "/");
  if (leadingthreefollowingslashes == start && start != begin)// This maybe doesn't work ;(
  {
    statuscode_ = BADREQUEST;
    request_type_ = RT_ERROR;
    return true;
  }
  else if (start == end) // FYI case "http://hostname" or "hostname" not trailing '/'
  {
    header_[" resource"] = "/";
    return true;
  }
  start = findVector(start, end, "/");
  std::vector<char>::iterator  tkn_end = findVector(start, end, "?");
  if (tkn_end == end)
    tkn_end = findVector(start, end, "#");
  header_[" resource"] = decodeURI(trim(replace(std::string(start, tkn_end), "//", "/")));
  if (resolvePath(getHeader(" resource").c_str()).empty()) // FYI Avoid Directory Transversal Vulnerability.
  {
    statuscode_ = FORBIDDEN;
    request_type_ = RT_ERROR;
    return true;
  }
  return header_[" resource"].size();
}

bool request::Handler::isResourceURI_v(std::vector<char>::iterator begin, std::vector<char>::iterator end)
{
  if (begin == end)
    return false;
  std::vector<char>::iterator start = findVector(begin, end, "//");
  std::vector<char>::iterator leadingthreefollowingslashes;
  if (start != end)
    start += 2;
  else
    start = begin;
  leadingthreefollowingslashes = start;
  start = findVector(start, end, "/");
  if (leadingthreefollowingslashes == start && start != begin)// This maybe doesn't work ;(
  {
    statuscode_ = BADREQUEST;
    request_type_ = RT_ERROR;
    return true;
  }
  else if (start == end) // FYI case "http://hostname" or "hostname" not trailing '/'
  {
    header_[" resource"] = "/";
    return true;
  }
  start = findVector(start, end, "/");
  std::vector<char>::iterator  tkn_end = findVector(start, end, "?");
  if (tkn_end == end)
    tkn_end = findVector(start, end, "#");
  header_[" resource"] = decodeURI(trim(replace(std::string(start, tkn_end), "//", "/")));
  if (resolvePath(getHeader(" resource").c_str()).empty()) // FYI Avoid Directory Transversal Vulnerability.
  {
    statuscode_ = FORBIDDEN;
    request_type_ = RT_ERROR;
    return true;
  }
  return header_[" resource"].size();
}

bool request::Handler::isProtocol_v(std::vector<char>::iterator begin,
    std::vector<char>::iterator end)
{
  if (begin == end)
    return false;
  std::vector<char>::iterator tkn_end = findVector(begin, end, "://");
  if (tkn_end == end)
    return false;
  std::vector<char>::iterator tkn_begin = begin;
  if (!::isalpha(*begin))
  {
    statuscode_ = BADREQUEST;
    request_type_ = RT_ERROR;
    return false;
  }
  header_[" protocol"] = std::string(tkn_begin, tkn_end);
  while (begin != tkn_end)
  {
    if (!(validChar("-.+", *begin) || ::isalnum(*begin)))
    {
      std::cout << "In this string \"" << header_[" protocol_"] << "\" the first invalid character is '" << *begin << "'" << std::endl;
      statuscode_ = BADREQUEST;
      request_type_ = RT_ERROR;
      return false;
    }
    begin++;
  }
  return true;
}


bool request::Handler::isResource_v(std::vector<char>::iterator begin, std::vector<char>::iterator end) {
  if (begin == end)
    return false;
  if (*begin == '/')
  {
    isResourcenotURI_v(begin, end);
    isParams_v(begin, end);
  }
  else
    return (isURI_v(begin, end));
  return true;
}

// Host
bool request::Handler::isHost_v(std::vector<char>::iterator begin, std::vector<char>::iterator end)
{
  if (begin == end)
    return false;
  std::vector<char>::iterator start = begin; 
  while (start != end)
  {
    if (!(validChar("-._~!$&\\\"*+,;=:\t ", *start) || ::isalnum(*start)))
      return false;
    start++;
  }
  header_[" host"] = trim(toLower(std::string(begin, end)));
  setTargetServer(getHeader(" host"));
  if (targetserver_ && !targetLocation_)
  {
    setTargetLocation(getHeader(" resource"));
  }
  return true;
}

bool request::Handler::containsNonDigit(const std::string& str)
{
    return std::find_if(str.begin(), str.end(), std::not1(std::ptr_fun(::isdigit))) != str.end();
}

bool request::Handler::isIpv4_v(std::vector<char>::iterator begin, std::vector<char>::iterator end)
{
  if (begin == end) // If invalid host error 400 BadRequest
    return false;
  std::stringstream validAddress(std::string(begin, end));
  std::string       byte;
  int counter = 0;
  while (std::getline(validAddress, byte, '.'))
  {
    if (containsNonDigit(byte) || std::atoi(getCharPtr(byte)) < 0
        || std::atoi(getCharPtr(byte)) > 255 || byte.empty()
        || (byte.length() > 1 && byte[0] == '0'))
      return false;
    counter++;
  }
  header_["host"] = trim(toLower(std::string(begin, end)));
  return (counter == 4);
}

bool request::Handler::isUserinfo_v(std::vector<char>::iterator begin,
    std::vector<char>::iterator end)
{
  if (begin == end)
    return false;
  std::vector<char>::iterator tkn_end = findVector(begin, end, "@");
  if (tkn_end == end) 
    return false;
  std::vector<char>::iterator start  = findVector(begin, tkn_end, "//");
  if (start == tkn_end)
    start = begin;
  else
    start += 2;
  return std::string(start, tkn_end).size();
}

bool request::Handler::isHostURI_v(std::vector<char>::iterator begin,
    std::vector<char>::iterator end)
{
  if (begin == end)
    return false;
  std::vector<char>::iterator start = findVector(begin, end, "@");
  if (start == end) 
  {
    start = findVector(begin, end, "//");
    if (start == end)
      start = begin;
    else
      start += 2;
  }
  else // userinfo == 400 on nginx with my conf.
    start++;
  std::vector<char>::iterator  tkn_end;
  tkn_end = findVector(start, end, ":");
  if (tkn_end == end)
    tkn_end = findVector(start, tkn_end, "/");
  header_["host"]= trim(toLower(std::string(start, tkn_end)));
  return (isIpv4_v(start, tkn_end) || isHost_v(start, tkn_end));
}

bool request::Handler::isURI_v(std::vector<char>::iterator begin, std::vector<char>::iterator end)
{
  if (begin == end)
  {
    statuscode_ = BADREQUEST;
    return false;
  }
  if (!isProtocol_v(begin, end))
  {
    std::cout << "Nginx give an error" << std::endl;
    statuscode_ = BADREQUEST;
    return false;
  }
  isUserinfo_v(begin, end);
  if (!isHostURI_v(begin, end))
  {
    statuscode_ = BADREQUEST;
    return false;
  }
  isPortURI_v(begin, end);
  isResourceURI_v(begin, end);
  isParams_v(begin, end);
  return true;
}

//Port
bool request::Handler::isPort_v(std::vector<char>::iterator begin, std::vector<char>::iterator end)
{
  if (begin == end)
  {
    statuscode_ = BADREQUEST;
    return false;
  }
  std::stringstream validPort(std::string(begin, end));
  int port_nbr;
  validPort >> port_nbr;
 // Not end of file // Overflow.
  if (!validPort.eof() || validPort.fail())
  {
    statuscode_ = BADREQUEST;
    return false;
  }
 //if (!(port_nbr > 0 && port_nbr <= MAX_PORT_NBR)) // I think unecesarry for my config
 //  return false;
  header_[" port"] = trim(toLower(std::string(begin, end)));
  return true;
}

bool request::Handler::isPortURI_v(std::vector<char>::iterator begin, std::vector<char>::iterator end)
{
  std::vector<char>::iterator start = findVector(begin, end, "//");
  if (start == end)
    start = begin;
  else
    start += 2;
  start = findVector(start, end, ":");
  if (start == end)
    return false; // default port
  std::vector<char>::iterator tkn_end = findVector(++start, end, "/");
  return (isPort_v(start, tkn_end));
}

// Decode not convertible to vector at least an easy way.
std::string request::Handler::decodeURI(const std::string &uri)
{
  std::string decode;
  size_t start = 0;
  size_t old_start = 0;
  int decoded;
  while (uri.find('%', start) != std::string::npos)
  {
    start = uri.find('%', start);
    decode += uri.substr(old_start, start - old_start);
    if (start + 2 >= uri.size())
      return ""; // Error
    decoded = hexValue(uri.substr(start + 1, 2));
    if (decoded == -1)
      return ""; // Error
    decode += static_cast<char>(decoded);
    start += 3;
    old_start = start;
  }
  decode += uri.substr(start, -1);
  while (decode.find('+') != std::string::npos)
    decode[decode.find('+')] = ' ';
  return decode;
}

// Is fine the type int is for error handling
int request::Handler::hexValue(std::string coded)
{
    std::string hex = "0x";
    hex += coded;
    std::istringstream converter(hex);
    int value;
    converter >> std::hex >> value;
    if (converter.fail() || !converter.eof())
    {
      std::cout << "converter fail: " << converter.fail() << " eof: " << converter.eof() << std::endl;
      return -1;
    }
    return value;
}

bool request::Handler::isencoded_v(std::vector<char>::iterator it, std::vector<char>::iterator end)
{
  if (it == end || *it != '%')
    return false;
  it++;
  if (it == end || std::isxdigit(*it))
    return false;
  it++;
  return (it != end && !std::isxdigit(*it));
}


std::string request::Handler::getEndOfLine_v(const std::vector<char>::iterator begin,
const std::vector<char>::iterator end)
{
  std::vector<char>::iterator found = findVector(begin, end, "\r\n");
  if (found != end)
    return "\r\n";
  found = findVector(begin, end, "\n");
  if (found != end)
    return "\n";
  return "";
}


char *request::Handler::getCharPtr(std::string &string) // == string.begin()
{
  return (&string[0]);
}

//bool request::Handler::startsWith(std::vector<char>::iterator begin, std::vector<char>::iterator end, std::string needle)
//{
// return (std::search(begin, end, needle.begin(), needle.end()) == begin);
//}
//bool request::Handler::validChar(std::vector<char>::iterator begin, std::vector<char>::iterator end, char valid)
// return (valid_chars.find(check) != std::string::npos);
bool request::Handler::validChar(std::string valid_chars, char check)
{
  return (valid_chars.find(check) != std::string::npos);
}
