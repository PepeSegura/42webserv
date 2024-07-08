#include "webserv.hpp"
#include "Ideletefile.hpp"
#include "deletefile.hpp"

void network::DeleteFile::DeteleFile(){}

network::DeleteFile::~DeleteFile(){}

bool network::DeleteFile::deleteFile(const std::string &abs_path)
{
    if (remove(abs_path.c_str()) != 0)
        return false;
    return true;
}
