#include "cgimanager.hpp"

cgi::CgiManager::CgiManager(std::vector<std::string> params, std::deque<char> req_body, std::map<std::string, std::string> env_vars)
{
    _vec_args = params;
    _req_body = req_body;
    _env_map = env_vars;
}

cgi::CgiManager::~CgiManager()
{
}

std::deque <char> cgi::CgiManager::generateResponse(request::Handler& header_hander)
{
    std::deque<char> response;
    try
    {
        char c;

        exec();
        while(read(_pipe[READ], &c, 1) > 0)
            response.push_back(c);
        header_hander.includeHeader(response, true, "", "text/html");
        close(_pipe[READ]);
    }
    catch(const CgiException& e)
    {
        e.error();
        _exit_status = 500;
        header_hander.includeHeader(response, false, "", "text/html");
    }
    return response;
}

void cgi::CgiManager::exec()
{
    if (pipe(_pipe) == -1)
        throw CgiException("Pipe failed " + std::string(strerror(errno)));
   else if ((_childpid = fork()) == -1)
        throw CgiException("Fork failed " + std::string(strerror(errno)));
    else if (_childpid == 0)
        exeChild();
    else
        waitChild();
}

void cgi::CgiManager::exeChild()
{
    try
    {
        char **env;
        char **args;
        std::string directory;

        close(_pipe[READ]);
        if (dup2(_pipe[WRITE], STDOUT_FILENO) == -1)
            throw CgiException("Dup failed " + std::string(strerror(errno)));
        close(_pipe[WRITE]);
        directory = getDirectory(_vec_args[0]);
        if (chdir(directory.c_str()))
            throw CgiException("Chdir failed " + std::string(strerror(errno)));
        else if (_req_body.size() > 0)
            writeToStdin();
        env = setCgiEnv(_env_map);
        args = setCgiArgs(_vec_args);
        execve(args[0], args, env);
        exit(EXIT_FAILURE);
    }
    catch(const CgiException& e)
    {
        e.error();
        exit(EXIT_FAILURE);
    }
}

void cgi::CgiManager::waitChild()
{
    close(_pipe[WRITE]);
    time_t start_time = time(NULL);
    while (time(NULL) < start_time + CGI_TIMEOUT)
    {
        if (waitpid(_childpid, &_exit_status, WNOHANG) != 0 && WIFEXITED(_exit_status))
        {
            _exit_status = WEXITSTATUS(_exit_status);
            if (_exit_status != 0)
                throw CgiException("Child Execution failed");
            return;
        }
        usleep(1000);
    }
    kill(_childpid, SIGKILL);
    throw CgiException("Child Execution timeout");
}


std::map<std::string, std::string> cgi::CgiManager::headerToCgiEnv(std::map<std::string, std::string>& env_map)
{
    std::map<std::string, std::string> new_map;
    for (std::map<std::string, std::string>::iterator it = env_map.begin(); it != env_map.end(); ++it)
    {
        std::string key = it->first;
        std::string value = it->second;
        std::replace(key.begin(), key.end(), '-', '_');
        std::transform(key.begin(), key.end(), key.begin(), ::toupper);
        if (keyIsHeadField(key))
            key = "HTTP_" + key;
        new_map[key] = value;
    }
    return new_map;
}

bool cgi::CgiManager::keyIsHeadField(std::string &key)
{
    if (key == "CONTENT_TYPE" || key == "CONTENT_LENGTH" || key == "GATEWAY_INTERFACE" || \
    key == "PATH_INFO" || key == "PATH_TRANSLATED" || key == "QUERY_STRING" || key == "REMOTE_ADDR" || \
    key == "REMOTE_IDENT" || key == "REMOTE_USER" || key == "REQUEST_METHOD" || key == "REQUEST_URI" || \
    key == "SCRIPT_NAME" || key == "SERVER_NAME" || key == "SERVER_PORT" || key == "SERVER_PROTOCOL" || key == "SERVER_SOFTWARE")
        return false;
    return true;
}

void cgi::CgiManager::writeToStdin()
{
    int temp_pipe[2];
    pipe(temp_pipe);
    if (_req_body.size() > MAX_PIPE_BUFF)
        throw CgiException("Body size too large");
    while(_req_body.size() > 0)
    {
        write(temp_pipe[WRITE], &_req_body.front(), 1);
        _req_body.pop_front();
    }
    close(temp_pipe[WRITE]);
    dup2(temp_pipe[READ], STDIN_FILENO);
    close(temp_pipe[READ]);
}

char **cgi::CgiManager::setCgiEnv(std::map<std::string, std::string> &env_map)
{
    env_map = headerToCgiEnv(env_map);
    char** new_env = new char*[env_map.size() + 1];

    int i = 0;
    for (std::map<std::string, std::string>::const_iterator it = env_map.begin(); it != env_map.end(); ++it)
    {
        const std::string concatenated = it->first + "=" + it->second;
        new_env[i] = new char[concatenated.size() + 1];
        std::strncpy(new_env[i], concatenated.c_str(), concatenated.size() + 1);
        ++i;
    }
    new_env[env_map.size()] = NULL;
    return new_env;
}

char **cgi::CgiManager::setCgiArgs(std::vector<std::string>& vec_args)
{
    for (std::vector<std::string>::iterator it = vec_args.begin(); it != vec_args.end(); ++it)
    {
        if (it == vec_args.begin() && access(it->c_str(), X_OK) != 0)
            throw CgiException("File " + *it + " is not executable " + std::string(strerror(errno)));
        else if (access(it->c_str(), R_OK) != 0)
            throw CgiException("File " + *it + " is not readable " + std::string(strerror(errno)));
    }
    return vecToMatrix(vec_args);
}

char **cgi::CgiManager::vecToMatrix(std::vector<std::string> &vec)
{
    char **matrix = new char*[vec.size() + 1];
    for (size_t i = 0; i < vec.size(); ++i) {
        matrix[i] = new char[vec[i].size() + 1];
        std::strncpy(matrix[i], vec[i].c_str(), vec[i].size() + 1);
    }
    matrix[vec.size()] = NULL;
    return matrix;
}


std::string cgi::CgiManager::getDirectory(std::string &abs_cgipath)
{

    size_t found = abs_cgipath.find_last_of("/");
    if (found == std::string::npos)
        throw CgiException("Could not find directory of cgi file");
    return (abs_cgipath.substr(0, found));
}

cgi::CgiManager::CgiException::CgiException(const std::string &msg)
{
    _msg = msg;
}

cgi::CgiManager::CgiException::~CgiException() throw()
{
}

void cgi::CgiManager::CgiException::error() const throw()
{
    ConsoleLog::print(ConsoleLog::ERROR, "CgiException: %s", _msg.c_str());
}
