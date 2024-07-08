#pragma once

#include "webserv.hpp"

namespace network
{
    class DeleteFile 
    {
        public:
            ~DeleteFile();
            static bool    deleteFile(const std::string &abs_path);
        protected:
            void DeteleFile();
    };
}
