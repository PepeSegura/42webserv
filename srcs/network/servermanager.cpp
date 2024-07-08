#include "servermanager.hpp"


network::ServerManager::ServerManager(){}

network::ServerManager::~ServerManager(){}

int network::ServerManager::is_running = 1;

void network::ServerManager::run(Servers const &servers)
{
	if (connections.initServConnections(servers) == false)
		return;
    struct pollfd *fd_conn = connections.getPollFd();
	while (is_running)
	{
		int current_size = connections.getSize();
		int num_events = poll(fd_conn, current_size, POLL_TIMEOUT);
		if (!is_running)
			break;
		if (num_events < 0)
			throw std::runtime_error("poll() system call error");
		for (int i = 0; i < current_size && num_events > 0; i++)
		{
            if(connections.eventInSocket(i) == true)
				num_events--;
		}
		connections.checkTimeout();
        connections.zipConnections();
	}
}

void network::ServerManager::catchSignals()
{
	std::signal(SIGINT, ServerManager::endServerManager);
	std::signal(SIGTERM, ServerManager::endServerManager);
	std::signal(SIGPIPE, SIG_IGN);
}

void network::ServerManager::endServerManager(int const signal)
{
	std::cout << std::endl;
	if (signal == SIGINT)
	{
		ConsoleLog::print(ConsoleLog::EXIT, "Closing WEBSERV after SIGINT");
		is_running = 0;
	}
	else if (signal == SIGTERM)
	{
		ConsoleLog::print(ConsoleLog::EXIT, "Closing WEBSERV after SIGTERM");
		is_running = 0;
	}
}
