
namespace network
{
    class IDeleteFile
    {
        public:
            virtual ~IDeleteFile() {}
            virtual bool deleteFile(const std::string &abs_path) = 0;
    };
}
