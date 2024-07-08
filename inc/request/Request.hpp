// Copyright 2024 42Madrid GPL
// Author: alvjimen
#ifndef REQUEST_HPP_
# define REQUEST_HPP_
# include <sys/stat.h>
# include <unistd.h>
# include <iostream>
# include <cctype>
# include <string>
# include <sstream>
# include <cstring>
# include <vector>
# include <map>
# include <deque>
# include <set>
# include <algorithm>
# include <fstream>
# include <ctime>
# include <dirent.h>
# include "socket.hpp"
# include "deletefile.hpp"
# include "Server.hpp"

namespace request {
  typedef enum e_variety
  {
    MAX_PORT_NBR = 65535
  } t_variety;

  typedef enum e_request_type
  {
    RT_ERROR,
    GET,
    POST,
    POSTGET,
    DELETE,
    DELETEGET,
    DELETEPOST,
    DELETEPOSTGET,
    RT_INCOMPLETE
  } t_request_type;

  typedef enum e_end_of_line
  {
    EOL_ERROR,
    CRLF,
    LF
  } t_end_of_line;

  typedef enum e_request_status
  {
    RS_NOEOL,
    RS_FIRSTLINE,
    RS_BODY,
    RS_END // This could be for an error or an fine status.
  } t_request_status;


  class Handler // Begin Abstract Class Handler
  {
    public:
      // Canonical not implemented 100% compliance not operator= and reference.
      Handler(void);
      Handler(std::vector<const ServerConfig*> servers);
      Handler&	operator=(Handler const & rhs);
      Handler(Handler const & src);
      ~Handler(void);

      //test.
      static bool isParameter(std::string type);
      static void debug_Location(parser::Location *location);
      static void sprintVector(std::vector<std::string> const &vector);

      // For future Interface HEADER.
      static std::string getContentType(std::string extension);
      static std::string getStatusMSG(int statuscode);
      static bool isStatuscodeRedirect(int statuscode);
      static std::string getDate(void);
      static std::deque<char> createHeaders(int statuscode, std::string version, bool keepalive, std::string contenttype, size_t size, std::string customstr, std::deque<char> &headers);
      static std::deque<char> getDeleteError(int statuscode); //
      static std::deque<char>::iterator findDeque( std::deque<char>::iterator begin, std::deque<char>::iterator end, std::string search);

      bool includeHeader(std::deque<char> & response, bool  status, std::string customstr ,  std::string contentType);// customstr just in case u want a cookie or a customHeader for some reason.

      bool processRequest(char *buffer, size_t num_bytes);
      std::deque<char> responseRequest(void);

      /* Getters */
      std::deque<char> getResponse(void);
      bool getKeepalive();
      std::string getHeader(std::string headerkey);
      std::vector<std::string> *getVector(std::string headerkey);
      std::deque<char> const & getBody(void) const;
      std::string const & getParamsString() const; // INFO QUERY_STRING
      std::vector<std::string> const & getParamsSplit() const;
      std::string getCGIPath();
      std::string getCGIExtension();
      const ServerConfig *getTargetserver();
      const parser::Location *getTargetLocation();
      std::ostream& printVector(std::ostream& COUT);
      std::ostream& printHeader(std::ostream& COUT);
      std::string getUploadStore() const;
      std::string getUploadPass() const;
      void printHeader();
      /*This shouldn't be public*/
      std::string resolvePath(const std::string path);
      void        reset();

    private:
			std::string							upload_store_;
			std::string							upload_pass_;
      std::vector<const ServerConfig*> servers_;
      std::vector<parser::Location> locations_;
      const ServerConfig *targetserver_;
      parser::Location *targetLocation_;
      parser::Location targetlocation_;
      std::map<std::string, std::vector<std::string> > vector_; //params, cookies, Content-Encoding
      std::map<std::string, std::string>  header_;
      std::vector<char> request_;
      std::deque<char> body_;
      std::string resource_;
      int request_status_;
      int request_type_;
      int statuscode_;
      bool customreturn_;
      std::vector<char>::iterator line_start_;
      std::vector<std::deque<char> > multipart_;
      std::vector<std::string> files_;
      std::string eol_;
      // Private static methods.

      bool isResourcenotURI_v(std::vector<char>::iterator begin, std::vector<char>::iterator end);
      static bool checkpart(std::deque<char> &part, std::string delim);
      static bool checkMultipart(std::vector<std::deque<char> > multipart, std::string delim);
      static std::string replaceBegining(std::string &src, std::string query, std::string replace);
      static std::string getIndex(std::string resource, std::vector<std::string> *index);
      static std::string getExtension(std::string resource);
      static bool isValidPath(const char* paht);
      static bool isFile(const char *path);
      static bool isDirectory(const char* path);
      template <typename T>
      static std::string to_string(T value);
      static std::string getDefaultError(int key);
      std::deque<char>::iterator findDeques(std::deque<char>::iterator begin, std::deque<char>::iterator end, std::deque<char>::iterator sbegin, std::deque<char>::iterator send);
      // Private Methods.
      bool writeFiles(std::deque<char> & part);
      bool handleUploadStore();
      std::string getFilename(std::deque<char> & part);
      bool isFilePart(std::deque<char> & part, int & status);
      std::string getValidFilename(std::string filename);
      bool validPart(std::deque<char> & part);
      std::vector<std::deque<char> > getMultiParts(std::deque<char> original, std::string delim);
      std::string debug_Cookie();
      void setAllowedMethod(std::string &allowedMethod);
      bool handlerUpload(std::string & resource);
      bool processRequest(std::vector<char> &request);
      void applyRootOrAlias(std::string &resource, std::string uri, std::string root);
      void setAllowedMethod(int &allowedMethod);
      void initVars(std::map<int, std::string> &custom_errors);
      size_t getBMS();
      void directoryListing();
      void setCGI();
      std::string getDocumentURI();
      void setCustomErrors(std::map<int, std::string> &custom_errors);
      void setReturn();
      bool setTargetServer(std::string host);
      bool setTargetLocation(std::string resource);
      std::deque<char> customMsg(std::string &str, std::deque<char> &response);
      std::string getResourceAbsolutePath(std::string &resource, std::vector<std::pair<std::string, std::string> > &locations);
      std::vector<std::string> parseURLParameters(std::string url);
      std::vector<std::string> parseContentTypeParameters(std::string url);
      bool invalidStatus(std::deque<char> &content, std::string &resource, std::string custom_error, std::string & contentType);
      std::deque<char> handleRedirection(std::string &str, std::deque<char> &response);
      void validStatus(std::string &resource, std::deque<char> &content, std::string &contentType);
      void handlerMethod(std::string &request);
      bool handleCGI(std::vector<std::string> *cgipath, std::vector<std::string> *cgi, std::string extension, std::string resource, int &statuscode, std::string &root);
      bool handleCustomErrors(std::vector<std::string> *index, std::string root, std::string &resource, std::vector<std::string> *cgi, int & status, int request_type, std::deque<char> & content);
      void handleDelete(std::string const & resource, int &statuscode);
      std::deque<char> getDir(const std::string &path);
      std::deque<char> readDirectoryEntries(DIR *dir, std::string const path);
      std::deque<char> getFile(std::string filename);
      std::string handlerResource(std::vector<std::string> *index, std::string root,
          std::string &resource, std::vector<std::string> *cgi, int &statuscode, int request_type);
      bool isCGI(std::vector<std::string> *cgipath, std::vector<std::string> *cgi, std::string extension, std::string resource, std::string &root);
      std::string getDocumentRoot();
      bool debug_info(bool value, std::string OK, std::string KO);
      bool isSameLenght(std::vector<char>::iterator begin, std::vector<char>::iterator end, std::string const & search);
      bool isContentTypeMultipart(std::string type);
      bool isBoundary(std::string tkn);
      bool isBchar(char cmp);
      bool isDelim(std::string const delimiters, char cmp);
      bool isField_content(char content);
      bool isField_vchar(char vchar);
      bool isObs_Text(char tchar);
      bool isVchar(char vchar);
      bool isSp(char sp);
      bool isContentType(std::string type);
      static bool        isToken(std::string token); // token == Field name // Host
      void        addVectorFromCharPtr(std::vector<char> &request, const char *pointer);
      int         hexValue(std::string coded);
      std::string decodeURI(const std::string &uri);
      static bool        validChar(std::string valid_chars, char check);
      bool        validPort(std::string port);
      char       *getCharPtr(std::string &string);
      bool        isRequestLine(std::string line);
      static bool        isTchar(char tchar);
//      bool        isToken(std::string token); // token == Field name // Host
      bool        isPreviousHeader(std::vector<char> &request,std::string &eol, std::vector<char>::iterator &line_end );
      bool        isHeaderField(std::string line);
      void        addMultiHeader(std::string key, std::string value);

      // Request Utilities
      bool isToken_v(std::vector<char>::iterator begin, std::vector<char>::iterator end);
      bool isField_value_v(std::vector<char>::iterator begin, std::vector<char>::iterator end);
      bool isMethod_v(std::vector<char>::iterator begin, std::vector<char>::iterator end);
      bool isProtocol_v(std::vector<char>::iterator begin,
          std::vector<char>::iterator end);
      std::string getEndOfLine_v(std::vector<char>::iterator begin,
          std::vector<char>::iterator end);
      bool isParams_v(std::vector<char>::iterator begin,
          std::vector<char>::iterator end);
      bool isResourceURI_v(std::vector<char>::iterator begin, std::vector<char>::iterator end);
      bool isHost_v(std::vector<char>::iterator begin, std::vector<char>::iterator end);
      bool isencoded_v(std::vector<char>::iterator it, std::vector<char>::iterator end);
      bool isIpv4_v(std::vector<char>::iterator begin, std::vector<char>::iterator end);
      bool containsNonDigit(const std::string& str);
      bool isHostURI_v(std::vector<char>::iterator begin, std::vector<char>::iterator end);
      bool isUserinfo_v(std::vector<char>::iterator begin,
          std::vector<char>::iterator end);
      bool isURI_v(std::vector<char>::iterator begin, std::vector<char>::iterator end);
      bool isPort_v(std::vector<char>::iterator begin, std::vector<char>::iterator end);
      bool isPortURI_v(std::vector<char>::iterator begin, std::vector<char>::iterator end);

      bool isRequestLine_v(std::vector<char>::iterator begin, std::vector<char>::iterator end);
      bool isHeaderField_v(std::vector<char>::iterator begin, std::vector<char>::iterator end);
      bool isHeader_v(std::vector<char> &request);
      static std::string trim(const std::string& str);
      static std::string toLower(const std::string& str);

    // Vector<char>
      std::vector<char>::iterator getNextToken(
          std::vector<char>::iterator &begin, std::vector<char>::iterator end,
          std::string const delimiters);
      // Error check if begin == end No more tokens left.
      bool isStringInsideVector(std::string keyname, std::string search);
      bool containsVector(std::vector<char>::iterator begin, std::vector<char>::iterator end, std::string search);
      bool handleEmptyToken(std::vector<char>::iterator &line_end, const std::string &eol);
      bool handleBeyondHeader(std::vector<char> &request, std::vector<char>::iterator &line_start, const std::string &eol);
      bool processToken(std::vector<char>::iterator line_end);
      bool isBeyondHeader(std::vector<char> & request, std::vector<char>::iterator line_start, std::string eol);
      bool equalVector(std::vector<char>::iterator begin, std::vector<char>::iterator end, std::string search);
      bool isParams(std::string uri);
      std::vector<char>::iterator findVector(
          std::vector<char>::iterator begin, std::vector<char>::iterator end,
          std::string search);
      bool isHttpVersion_v(std::vector<char>::iterator begin,
          std::vector<char>::iterator end);
      bool isResource_v(std::vector<char>::iterator begin, std::vector<char>::iterator end);
      bool isBody(std::vector<char> request);
      void addStringToVector(std::vector<std::string> &vector, std::string str);
      bool parseContentLenght(std::vector<char>::iterator begin, std::vector<char>::iterator end);
      bool parseChunkedBody(std::vector<char>::iterator &line_end, std::vector<char>::iterator end);
      void trailLines(std::vector<char>::iterator &line_begin, std::vector<char>::iterator end, std::string eol);
  }; // end of class handler

  typedef enum e_status_code
  {
    // Information responses.
    CONTINUE = 100,
    SWITCHINGPROTOCOLS = 101,
    PROCESSING = 102,
    EARLYHINTS = 103,
    // Sucesffull messages
    OK = 200,
    CREATED = 201,
    Accepted = 202,
    NON_AUTHORITATIVEINFORMATION = 203,
    NOCONTENT = 204,
    RESETCONTENT = 205,
    PARTIALCONTENT = 206,
    MULTI_STATUS = 207,
    ALREADYREPORTED = 208,
    IMUSED = 226,
    // Redirection messages
    MULTIPLECHOICES = 300,
    MOVEDPERMANENTLY = 301,
    FOUND = 302,
    SEEOTHER = 303,
    NOTMODIFIED = 304,
    TEMPORYREDIRECT = 307,
    PERMANENTREDIRECT = 308,
    // Client error responses
    BADREQUEST = 400,
    UNATHORIZED = 401,
    PAYMENTREQUERED = 402,
    FORBIDDEN = 403,// Permission error couldn't read | write | execute.
    NOTFOUND = 404, // File not found
    METHODNOTALLOWED = 405,
    NOTACCEPTABLE = 406,
    PROXYAUTHENTICATIONREQUIRED = 407,
    REQUESTTIMEOUT = 408,
    CONFLICT = 409,
    GONE = 410,
    LENGTHREQUIRED = 411,
    PRECONDITIONFAILED = 412,
    CONTENTTOOLARGE = 413, // ERROR on BODY LENGTH
    URITOOLONG = 414, // ERROR on URI length
    UNSUPPORTEDMEDIATYPE = 415,
    RANGENOTSATISFIABLE = 416,
    EXPECTATIONFAILED = 417,
    IAMTEAPOT = 418,
    MISDIRECTEDREQUEST = 421,
    UNPROCESSABLECONTENT = 422,
    LOCKED = 423,
    FAILEDDEPENDENCY = 424,
    TOOEARLY = 425,
    UPGRADEREQUIRED = 426,
    PRECONDITIONREQUIRED = 428,
    TOOMANYREQUEST = 429,
    REQUESTHEADERFIELDSTOOLARGE = 431,
    UNAVAILABLEFORLEGALREASONS = 451,
    //Server Errors
    INTERNALSERVERERROR = 500,
    NOTIMPLEMENTED = 501,
    BADGATEWAY = 502,
    SERVICEUNAVALAIBLE = 503,
    GATEWAYTIMEOUT = 504,
    HTTPVERSIONNOTSUPPORTED = 505,
    VARIANTALSONEGOTIATES = 506,
    NOTEXTENDED = 510,
    NETWORKAUTHENTICATIONREQUIRED = 511
  } t_status_code;

}// namespace request
std::ostream&	operator<<(std::ostream& COUT, request::Handler& location);
# include "Request.ipp"
#endif  // REQUEST_HPP_
