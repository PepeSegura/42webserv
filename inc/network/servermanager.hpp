#pragma once

#include "webserv.hpp"
#include "connections.hpp"
#include "deletefile.hpp"
#include "parser/Parser.hpp"

namespace network{
    class ServerManager {
        public:
            ServerManager();
            ~ServerManager();
            void run(Servers const &servs_config);
            void catchSignals();
        private:
            Connections connections;
            static void endServerManager(int const signal);
            static int is_running;
    };
}
