#include "clientsocket.hpp"

network::ClientSocket::ClientSocket()
{}

network::ClientSocket::ClientSocket(struct pollfd *new_poll_pointer, std::vector<const ServerConfig*> serv_config): _handlerRequest(request::Handler(serv_config))
{
	_serv_config = serv_config;
	_keep_alive = true;
	_poll_pointer = new_poll_pointer;
	_sock_fd = new_poll_pointer->fd;
	_timeout = time(NULL);
}

network::ClientSocket::~ClientSocket()
{
	close(_sock_fd);
}

int network::ClientSocket::getEvent() const
{
	return _poll_pointer->revents;
}

bool network::ClientSocket::getKeepAlive() const
{
    return _keep_alive;
}


int network::ClientSocket::sendResponse()
{
	int bytes_to_send;
	int clientfd = _poll_pointer->fd;
	std::vector <char> response;

	bytes_to_send = _response.size() > SR_BUFF ? SR_BUFF : _response.size();
	response = std::vector<char>(_response.begin(), _response.begin() + bytes_to_send);
	if (send(clientfd, response.data(), bytes_to_send, 0) <= 0)
		return -1;
	while (bytes_to_send > 0 && !_response.empty())
	{
		bytes_to_send--;
		_response.pop_front();
	}
	if (_response.empty())
		return 1;
	return 0;
}


int network::ClientSocket::recvRequest()
{
	ssize_t rec;
	char buff[SR_BUFF];
	int clientfd = _poll_pointer->fd;

	rec = recv(clientfd, buff, SR_BUFF, 0);
	if (rec <= 0)
		return -1;
	return _handlerRequest.processRequest(buff, rec);
}

void network::ClientSocket::prepareResponse()
{
	_response = _handlerRequest.getResponse();
	this->_poll_pointer->events = POLLOUT;
}


void network::ClientSocket::nextRequest()
{
	_poll_pointer->events = POLLIN;
	_response.clear();
	_handlerRequest.reset();
}

void network::ClientSocket::sendTest(std::deque<char> &response)
{
	std::string str = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 12\r\n\r\nHello World!";
	response.insert(response.end(), str.begin(), str.end());
}

void network::ClientSocket::setResponse(std::deque<char> &response)
{
	_response = response;
}
