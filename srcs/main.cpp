#include "servermanager.hpp"
#include "parser/Parser.hpp"
#include "Request.hpp"

static char *setInputFile(int argc, char **argv)
{
	char *input_file;
	if (argc == 1)
	{
		std::cout << "Launching default config." << std::endl;
		input_file = (char *)DEFAULT_CONFIG;
	}
	else if (argc == 2)
		input_file = argv[1];
	else
		throw(std::runtime_error("Usage: ./webserv [configuration file]"));
	return (input_file);
}

int main(int argc, char **argv)
{
	char *input_file;
	try
	{
		input_file = setInputFile(argc, argv);
		parser::Parser config_file(input_file);
		std::cout << config_file;
		network::ServerManager servmanager;
		servmanager.catchSignals();
		servmanager.run(config_file.getServers());
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		exit(EXIT_FAILURE);
	}
	return 0;
}
