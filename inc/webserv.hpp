#pragma once

#define MAX_CNCTS 2000
#define SERVER_CLNTS_LIMIT 100
#define SR_BUFF 1024
#define MAX_PIPE_BUFF 16384
// LIMITS
#define CGI_TIMEOUT 30
#define KEEPALIVE_TIMEOUT 75
#define POLL_TIMEOUT 60


#define DEFAULT_CONFIG "configs/default.conf"

#include "consolelog.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdexcept>
#include <unistd.h>
#include <cstring>
#include <poll.h>
#include <fcntl.h>
#include <vector>
#include <deque>
#include <map>
#include <cerrno>
#include <csignal>
#include <ctime>
#include <cstdlib>
#include <cctype>
#include <typeinfo>
#include <sys/wait.h>
#include <iomanip>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <functional>

std::string& ltrim(std::string& str);
std::string& rtrim(std::string& str);
std::string& trim(std::string& str);

