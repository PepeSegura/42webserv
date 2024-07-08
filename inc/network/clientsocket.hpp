#include "webserv.hpp"
#include "socket.hpp"
#include "Request.hpp"

namespace network{
	class ClientSocket : public Socket {
		public:
			ClientSocket(struct pollfd *new_poll_pointer, std::vector<const ServerConfig*> serv_config);
			~ClientSocket();
			int getEvent() const;
			bool getKeepAlive() const;
            int sendResponse();
            int recvRequest();
            void prepareResponse();
            void nextRequest();
			void setResponse(std::deque<char> &response);
		private:
			ClientSocket();
			//private methods
			request::Handler _handlerRequest;
            bool is_req_finish(int clientfd);
            void sendTest(std::deque<char> &response);
			//private members
            std::deque<char> _response;
			bool _keep_alive;

	};
}
