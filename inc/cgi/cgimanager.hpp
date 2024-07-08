#pragma once
#include "webserv.hpp"
#include "Request.hpp"

namespace cgi
{
    class CgiManager
    {
        enum epipes
        {
            READ = 0,
            WRITE = 1
        };
        public:
            CgiManager(std::vector<std::string> params, std::deque<char> req_body, std::map<std::string, std::string> env_vars);
            ~CgiManager();
            std::deque <char> generateResponse(request::Handler& header_hander);
            class CgiException : public std::exception
            {
                public:
                    CgiException(const std::string &msg);
                    virtual ~CgiException() throw();
                    void error() const throw();
                private:
                    std::string _msg;
            };
        private:
            void    writeToStdin();
            char**  setCgiEnv(std::map<std::string, std::string>& env_map);
            char**  setCgiArgs(std::vector<std::string>& vec_args);
            char**  vecToMatrix(std::vector<std::string>& vec);
            std::string    getDirectory(std::string& abs_cgipath);
            std::map<std::string, std::string>    headerToCgiEnv(std::map<std::string, std::string>& env_map);
            bool    keyIsHeadField(std::string& key);
            void    exec();
            void    exeChild();
            void    waitChild();
            int _pipe[2];
            int _exit_status;
            pid_t _childpid;

            std::vector<std::string> _vec_args;
            std::map<std::string, std::string> _env_map;
            std::deque<char> _req_body;
    };
}
