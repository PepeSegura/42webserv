#include "socket.hpp"



network::Socket::Socket()
{
	_poll_pointer = NULL;
}

network::Socket::~Socket(){}

int network::Socket::getFd() const
{
	return _sock_fd;
}

time_t network::Socket::getTimeout() const
{
	return _timeout;
}

void network::Socket::setFd(int const fd)
{
	_sock_fd = fd;
}

void network::Socket::updateTimeout()
{
	_timeout = time(NULL);
}

pollfd *network::Socket::getFdConnPointer()
{
	return _poll_pointer;
}

std::vector<const ServerConfig *> network::Socket::getServConfig() const
{
	return _serv_config;
}

void network::Socket::setFdConnPointer(pollfd *new_pointer)
{
	_poll_pointer = new_pointer;
}

void network::Socket::addServConfig(const ServerConfig* serv_config)
{
	_serv_config.push_back(serv_config);
}

network::Socket::SocketException::SocketException(const std::string &msg) : _msg(msg), _port(-1), _host(0){}

network::Socket::SocketException::SocketException(const std::string &msg, int const port, in_addr_t const host) : _msg(msg), _port(port), _host(host) {}

network::Socket::SocketException::~SocketException() throw()
{
}

void network::Socket::SocketException::error() const throw()
{
	if(_port == -1 )
		ConsoleLog::print(ConsoleLog::ERROR, "SocketException: %s", _msg.c_str());
	else
	{
		char *host = inet_ntoa(*(in_addr*)&_host);
		ConsoleLog::print(ConsoleLog::ERROR, "SocketException: %s - Address: %s Port: %d", _msg.c_str(), host, _port);
	}
}
