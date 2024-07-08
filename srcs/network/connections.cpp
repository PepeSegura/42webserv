#include "connections.hpp"

network::Connections::Connections(){}

network::Connections::~Connections()
{
	for (std::map<int, Socket *>::iterator it = _connections.begin(); it != _connections.end(); it++)
	{
		if (isServerSocket(it->second))
		{
			ServerSocket *to_delete = dynamic_cast<ServerSocket*>(it->second);
			ConsoleLog::print(ConsoleLog::EXIT, "Server Closed - Address: %s Port: %d", to_delete->getAddr().c_str(), to_delete->getPort());
			delete to_delete;
		}
		else
		{
			ClientSocket *to_delete = dynamic_cast<ClientSocket*>(it->second);
			ConsoleLog::print(ConsoleLog::EXIT, "Connection with Client %d closed at exit", to_delete->getFd());
			delete to_delete;
		}
	}
}

bool network::Connections::initServConnections(Servers const &vec_servs_config)
{
	try
	{
		ConsoleLog::print(ConsoleLog::INFO, "Initializing  WEBSERV...");
		bzero(_fd_conn, sizeof(_fd_conn));
		int index = 0;
		for (Servers::const_iterator serv_config = vec_servs_config.begin(); serv_config != vec_servs_config.end(); serv_config++)
		{
			if (index >= MAX_CNCTS)
				throw Socket::SocketException("Max number of socket connections reached");
			if (addVirtualIfDup(&(*serv_config)) == false)
			{
				ServerSocket *serv_socket = new ServerSocket(&_fd_conn[index], serv_config->getPort(), inet_addr(serv_config->getHost().c_str()));
				_fd_conn[index].fd = serv_socket->getFd();
				_fd_conn[index].events = POLLIN;
				serv_socket->addServConfig(&(*serv_config));
				_connections.insert(std::pair<int, Socket*>(serv_socket->getFd(), serv_socket));
				ConsoleLog::print(ConsoleLog::S_CONN, "New Server Created - Name: %s Fd: %i Address: %s Port: %d", serv_config->getServerName().c_str(), serv_socket->getFd(), serv_socket->getAddr().c_str(), serv_socket->getPort());
				index++;
			}
		}
		return true;
	}
	catch(const Socket::SocketException& e)
	{
		e.error();
		return false;
	}
}

pollfd *network::Connections::getPollFd()
{
	return _fd_conn;
}

int network::Connections::getSize() const
{
	return _connections.size();
}


bool network::Connections::eventInSocket(int const i)
{
	int poll_event = _fd_conn[i].revents;

	if (_fd_conn[i].fd < 0 || poll_event == 0)
		return false;
	else if (poll_event != POLLIN && poll_event != POLLOUT)
	{
		handleCloseConn((_connections.find(_fd_conn[i].fd))->second);
		return true;
	}
	Socket *ready_sock = (_connections.find(_fd_conn[i].fd))->second;
	if (isServerSocket(ready_sock))
		acceptNewConnections(dynamic_cast<ServerSocket&>(*ready_sock));
	else if (time(NULL) - ready_sock->getTimeout() <= KEEPALIVE_TIMEOUT)
		talkToClient(dynamic_cast<ClientSocket&>(*ready_sock));
	return true;
}

void network::Connections::zipConnections()
{
	int num_conn = _connections.size();
	for (int i = 0; i < num_conn; i++)
	{
		if (_fd_conn[i].fd == -1 && i < num_conn - 1)
		{
			_fd_conn[i] = _fd_conn[num_conn - 1];
			_fd_conn[num_conn - 1].fd = -1;
			num_conn--;
			if (num_conn > 1)
			{
				std::map<int, Socket*>::iterator to_mv = _connections.find(_fd_conn[i].fd);
				if (to_mv == _connections.end())
					return;
				to_mv->second->setFdConnPointer(&_fd_conn[i]);
			}
		}
	}
}

void network::Connections::handleCloseConn(Socket *to_delete)
{
	if (isServerSocket(to_delete))
		rmServerSocket(dynamic_cast<ServerSocket&>(*to_delete));
	else
	{
		ConsoleLog::print(ConsoleLog::C_CONN, "Connection with Client %d closed by client", to_delete->getFd());
		rmClientSocket(dynamic_cast<ClientSocket&>(*to_delete));
	}
}

void network::Connections::rmClientSocket(ClientSocket &client_sock)
{
	_connections.erase(client_sock.getFd());
	client_sock.getFdConnPointer()->fd = -1;
	delete &client_sock;
}

void network::Connections::rmServerSocket(ServerSocket &serv_sock)
{
	ConsoleLog::print(ConsoleLog::S_CONN, "Server Closed - Address: %s Port: %d", serv_sock.getAddr().c_str(), serv_sock.getPort());
	_connections.erase(serv_sock.getFd());
	serv_sock.getFdConnPointer()->fd = -1;
	delete &serv_sock;
}

void network::Connections::acceptNewConnections(ServerSocket &serv_sock)
{
	int new_client;
	int num_conn = _connections.size();

	while (num_conn < MAX_CNCTS && (new_client = accept(serv_sock.getFd(), NULL, NULL)) != -1)
	{
		fcntl(new_client, F_SETFL, O_NONBLOCK);
		_fd_conn[num_conn].fd = new_client;
		_fd_conn[num_conn].events = POLLIN;
		ClientSocket *client_sock = new ClientSocket(&_fd_conn[num_conn], serv_sock.getServConfig());
		this->_connections.insert(std::pair<int, Socket*>(new_client, client_sock));
		ConsoleLog::print(ConsoleLog::C_CONN, "New Client Fd: %d Connected to Server: %s in Port: %d", new_client, serv_sock.getAddr().c_str(), serv_sock.getPort());
		num_conn++;
	}
	if(errno != EAGAIN && errno != EWOULDBLOCK)
		ConsoleLog::print(ConsoleLog::ERROR, "Error accepting new connection on Server: %s in Port: %d", serv_sock.getAddr().c_str(), serv_sock.getPort());
}

void network::Connections::talkToClient(ClientSocket &client_sock)
{
	int is_msg_finish;
	int poll_event = client_sock.getEvent();

	if (poll_event == POLLIN)
		is_msg_finish = client_sock.recvRequest();
	else
		is_msg_finish = client_sock.sendResponse();
	if (is_msg_finish == -1)
		return handleCloseConn(&client_sock);
	else if (is_msg_finish)
	{
		if (poll_event == POLLIN)
			client_sock.prepareResponse();
		else if (poll_event == POLLOUT && client_sock.getKeepAlive())
			client_sock.nextRequest();
		else
			return handleCloseConn(&client_sock);
	}
	client_sock.updateTimeout();
}

bool network::Connections::isServerSocket(Socket *ready_sock) const
{
	if (typeid(*ready_sock) == typeid(ServerSocket))
		return true;
	return false;
}

bool network::Connections::addVirtualIfDup(const ServerConfig* new_serv_config)
{
	for (std::map<int, Socket *>::iterator it = _connections.begin(); it != _connections.end(); it++)
	{
		ServerSocket *serv_socket = dynamic_cast<ServerSocket*>(it->second);
		const ServerConfig *serv_config = serv_socket->getServConfig().front();
		if (serv_config->getHost() == new_serv_config->getHost() && serv_config->getPort() == new_serv_config->getPort())
		{
			serv_socket->addServConfig(new_serv_config);
			ConsoleLog::print(ConsoleLog::S_CONN, "New Virtual_Server Created - Name: %s Fd: %i Address: %s Port: %d", new_serv_config->getServerName().c_str(), serv_socket->getFd(), serv_socket->getAddr().c_str(), serv_socket->getPort());
			return true;
		}
	}
	return false;
}

void network::Connections::checkTimeout()
{
	std::vector<int> fd_to_delete;
	for (std::map<int, Socket *>::iterator it = _connections.begin(); it != _connections.end(); it++)
	{
		time_t last_action = it->second->getTimeout();
		if (last_action > 0 && time(NULL) - last_action > KEEPALIVE_TIMEOUT)
			fd_to_delete.push_back(it->first);
	}
	for (std::vector<int>::iterator it = fd_to_delete.begin(); it != fd_to_delete.end(); it++)
	{
		Socket *to_delete = (_connections.find(*it))->second;
		ConsoleLog::print(ConsoleLog::C_CONN, "Connection with Client %d closed by timeout", to_delete->getFd());
		rmClientSocket(dynamic_cast<ClientSocket&>(*to_delete));
	}
}
