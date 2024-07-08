#include "serversocket.hpp"

network::ServerSocket::ServerSocket()
{}

network::ServerSocket::ServerSocket(pollfd *new_poll_pointer, int const port, in_addr_t const host)
{
	_poll_pointer = new_poll_pointer; 
	_timeout = 0;
	int opt_val = 1;
	this->_m_addr.sin_family = AF_INET;
	this->_m_addr.sin_port = htons(port);
	this->_m_addr.sin_addr.s_addr = host;

	if ((_sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		throw SocketException("socket() system call failed", port, host);
	else if (fcntl(_sock_fd, F_SETFL, O_NONBLOCK) != 0)
		throw SocketException("fcntl() system call failed", port, host);
	else if (setsockopt(_sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val)) != 0)
		throw SocketException("setsockopt() system call failed", port, host);
	else if (bind(_sock_fd, (struct sockaddr *)&_m_addr, sizeof(_m_addr)) != 0)
		throw SocketException("bind() system call failed", port, host);
	else if (listen(_sock_fd, SERVER_CLNTS_LIMIT) != 0)
		throw SocketException("listen() system call failed", port, host);
}

network::ServerSocket::~ServerSocket()
{
	close(this->_sock_fd);
}

int network::ServerSocket::getPort() const
{
	return ntohs(this->_m_addr.sin_port);
}

std::string network::ServerSocket::getAddr() const
{
	return inet_ntoa(this->_m_addr.sin_addr);
}
