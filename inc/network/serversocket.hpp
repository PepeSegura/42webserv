#pragma once

#include "webserv.hpp"
#include "socket.hpp"

namespace network{
	class ServerSocket : public Socket {
		public:
			ServerSocket(pollfd *new_poll_pointer, int const port, in_addr_t const host);
			~ServerSocket();
			int getPort() const;
			std::string getAddr() const;
		private:
			ServerSocket();
			sockaddr_in _m_addr;
	};
}

