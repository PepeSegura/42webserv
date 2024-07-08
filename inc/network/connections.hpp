#pragma once

#include "webserv.hpp"
#include "serversocket.hpp"
#include "clientsocket.hpp"
#include "cgimanager.hpp"
#include "parser/Parser.hpp"


namespace network{
    class Connections{
        public:
            Connections();
            ~Connections();
        //public methods
            bool initServConnections(Servers const &servers);
            bool eventInSocket(int const index);
            void checkTimeout();
            void zipConnections();
        //getters
            struct pollfd* getPollFd();
            int getSize() const;
        private:
        //private methods
            void acceptNewConnections(ServerSocket &serv_sock);
            void talkToClient(ClientSocket &client_sock);
            void rmClientSocket(ClientSocket &client_sock);
            void rmServerSocket(ServerSocket &serv_sock);
            void handleCloseConn(Socket *to_delete);
            bool isServerSocket(Socket *connection) const;
            bool addVirtualIfDup(const ServerConfig* new_serv_config) ;
        //private members
            struct pollfd _fd_conn[MAX_CNCTS];
            std::map<int, Socket*> _connections;
    };
}
