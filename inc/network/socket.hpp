#pragma once
#include "webserv.hpp"
#include <Server.hpp>

typedef parser::Server ServerConfig;

namespace network{
	//Abstract class
	class Socket{
		public:
			Socket();
			virtual ~Socket() = 0;
			int getFd() const;
			time_t getTimeout() const;
			struct pollfd *getFdConnPointer();
			std::vector<const ServerConfig*> getServConfig() const;
			void setFd(int const fd);
			void setFdConnPointer(struct pollfd *fd_conn);
			void addServConfig(const ServerConfig* serv_config);
			void updateTimeout();
			class SocketException : public std::exception{
				public:
				 	SocketException(const std::string &msg);
				 	SocketException(const std::string &msg, int const port, in_addr_t const host);
					virtual ~SocketException() throw();
					void error() const throw();
				private:
					std::string _msg;
					int const _port;
					in_addr_t const _host;
			};
		protected:
			int _sock_fd;
			time_t _timeout;
			pollfd *_poll_pointer;
			std::vector<const ServerConfig*> _serv_config; 
	};
}
