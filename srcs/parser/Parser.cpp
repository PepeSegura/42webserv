/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: psegura- <psegura-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/12 19:23:08 by psegura-          #+#    #+#             */
/*   Updated: 2024/03/09 20:00:47 by psegura-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parser/Parser.hpp"

const std::string keywords[] = {
	"root",
	"accept_method",
	"return",
	"autoindex",
	"index",
	"client_max_body_size",
	"upload_store",
	"upload_pass",
	"listen",
	"server_name",
	"cgi_pass",
	"error_page",
	"alias",
	"NULL",
};

int	tokenIsKeyword(const std::string &token)
{
	for (size_t i = 0; keywords[i] != "NULL"; i++) {
		if (token == keywords[i])
			return (1);
	}
	return (0);
}

enum State {
	START,
	ERROR,
	SERVER,
	SERVER_OPEN,
	SERVER_CLOSE,
	LOCATION,
	LOCATION_OPEN,
	LOCATION_CLOSE,
	KEYWORD,
	PARAMETER,
	SEMICOLON,
	LOCATION_URI,
};

const std::string name_states[12] = {
	"START",
	"ERROR",
	"SERVER",
	"SERVER_OPEN",
	"SERVER_CLOSE",
	"LOCATION",
	"LOCATION_OPEN",
	"LOCATION_CLOSE",
	"KEYWORD",
	"PARAMETER",
	"SEMICOLON",
	"LOCATION_URI",
};

/* TABLE OF STATES */

int	getState(int x, int y)
{
	const int	states[][11] = {
 	{1, 2, 1, 1, 1, 1, 1, 1, 1,  1,  1}, //  0 -> START
 	{1, 1, 1, 1, 1, 1, 1, 1, 1,  1,  1}, //  1 -> ERROR
 	{1, 1, 3, 1, 1, 1, 1, 1, 1,  1,  1}, //  2 -> SERVER
 	{1, 1, 1, 1, 5, 1, 1, 8, 1,  1,  1}, //  3 -> SERVER_OPEN
 	{1, 2, 1, 1, 1, 1, 1, 1, 1,  1,  1}, //  4 -> SERVER_CLOSE
 	{1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 11}, //  5 -> LOCATION
 	{1, 1, 1, 1, 1, 1, 7, 8, 1,  1,  1}, //  6 -> LOCATION_OPEN
 	{1, 1, 1, 4, 5, 1, 1, 1, 1,  1,  1}, //  7 -> LOCATION_CLOSE
 	{1, 1, 1, 1, 1, 1, 1, 1, 9,  1,  1}, //  8 -> KEYWORD
 	{1, 1, 1, 1, 1, 6, 1, 1, 9, 10,  1}, //  9 -> PARAMETER
 	{1, 2, 1, 1, 5, 1, 7, 8, 1,  1,  1}, // 10 -> SEMICOLON
 	{1, 1, 1, 1, 1, 6, 1, 1, 1,  1,  1}, // 11 -> LOCATION_URI
	};

	return (states[x][y]);
}

int	chooseState(int state, const std::string& token)
{
	int	pos = 0;
	static int is_server;
	static int is_location;

	if (token == "server") {
		pos = 1; // SERVER
	}
	else if (token == "location") {
		pos = 4;
	}
	else if (token == "{") {
		if (state == SERVER)
		{
			pos = 2; // SERVER_OPEN
			is_server = 1;
		}
		if (state == LOCATION_URI)
		{
			pos = 5; // LOCATION_OPEN
			is_location = 1;
		}
	}
	else if (token == "}") {
		if ((state == SERVER_OPEN || state == LOCATION_CLOSE || state == SEMICOLON) && is_server)
		{
			pos = 3; //SERVER_CLOSE
			if (!is_location)
				is_server = 0;
		}
		if ((state == LOCATION_OPEN || state == SEMICOLON ) && is_location)
		{
			pos = 6; //LOCATION_CLOSE
			is_location = 0;
		}
	}
	else if (tokenIsKeyword(token)) {
		pos = 7; //KEYWORD
	}
	else if (token == ";") {
		pos = 9; //SEMICOLON
	}
	else
	{
		pos = 8; //PARAMETER
		if (state == LOCATION)
			pos = 10; //LOCATION_URI
	}
	state = getState(state, pos);
	// std::cerr << "POS: " << pos << " State: [" << name_states[state] << "] Token: [" << token << "]" << std::endl;
	return (state);
}

std::string expandEnv(std::string& input)
{
	std::string final = input;
    std::string	result;
    size_t		start_pos = 0;
    size_t		end_pos = 0;

    while (start_pos < final.size())
	{
        start_pos = input.find('$', start_pos);
        if (start_pos == std::string::npos)
            break;
        end_pos = start_pos + 1;
        while (end_pos < input.size() && std::isalnum(input[end_pos]))
            end_pos++;
        std::string var = input.substr(start_pos + 1, end_pos - start_pos - 1);
        const char* value = getenv(var.c_str());
        if (value)
		{
			std::string to_insert = value;
			final.erase(start_pos, end_pos - start_pos);
			final.insert(start_pos, to_insert);
            result += value;
			input = final;
        }
        start_pos = end_pos;
    }
    return final;
}

void parser::Parser:: removeComments(std::ifstream& config_file)
{
	std::string line;

	while (std::getline(config_file, line)) {
		if (line.empty())
			this->_config_file = this->_config_file + "\n";
		if (!line.empty())
		{
			size_t commentPos = line.find('#');
			if (commentPos != std::string::npos)
				line = line.substr(0, commentPos);
			std::string	cleanLine = rtrim(line);
			if (!cleanLine.empty())
				this->_config_file = this->_config_file + expandEnv(cleanLine) + "\n";
		}
	}
}

void parser::Parser:: storeTokens(void)
{
	std::istringstream iss(_config_file);
	std::string token;

	while (iss >> token) {
		if (token == ";")
			this->_tokens.push_back(token);
		else if (!token.empty() && token[token.length() - 1] == ';') {
			this->_tokens.push_back(token.substr(0, token.size() - 1));
			this->_tokens.push_back(";");
		} else
			this->_tokens.push_back(token);
	}
}

void parser::Parser:: evaluateTokens(void)
{
	int	state = 0;

	for (size_t i = 0; i < this->_tokens.size(); i++) {
		const std::string& token = this->_tokens[i];
		state = chooseState(state, token);
	}
	if (state != 4 && state != 10)
		throw (std::runtime_error("Error: Syntax error in the configuration file."));
}

void	removeBrackets(std::vector<std::string>& tokens)
{
	if (tokens.empty())
		return ;
	tokens.erase(tokens.begin());
	tokens.pop_back();
}

void parser::Parser:: splitIntoServerBlocks(void)
{
    std::vector<std::string> currentBlock;

    for (size_t i = 0; i < _tokens.size(); i++) {
        if (_tokens[i] == "server")
		{
            if (!currentBlock.empty())
			{
				removeBrackets(currentBlock);
                _server_blocks.push_back(currentBlock);
                currentBlock.clear();
            }
        }
		else
            currentBlock.push_back(_tokens[i]);
    }
	removeBrackets(currentBlock);
    if (!currentBlock.empty())
		_server_blocks.push_back(currentBlock);
}

std::ostream&	operator<<(std::ostream& COUT, parser::Parser& parser)
{
	Servers servers = parser.getServers();

	for (size_t i = 0; i < servers.size(); i++)
		COUT << servers[i];

	return (COUT);
}

void parser::Parser:: createServerConf(void)
{
	for (size_t i = 0; i < _server_blocks.size(); i++) {
		_servers.push_back(parser::Server(_server_blocks[i]));
	}
}
std::vector<parser::Server> parser::Parser:: getServers(void)
{
	return (_servers);
}

parser::Parser:: ~Parser() {}

void parser::Parser:: checkDuplicates(void)
{
	std::vector<std::string>	hosts;
	std::vector<size_t>			ports;
	std::vector<std::string>	names;

	for (size_t i = 0; i < _servers.size(); i++) {
		hosts.push_back(_servers[i].getHost());
		ports.push_back(_servers[i].getPort());
		names.push_back(_servers[i].getServerName());
	}

	for (size_t i = 1; i < _servers.size(); i++) {
		for (int j = i - 1; j >= 0; j--) {
			if (hosts[i] == hosts[j] && ports[i] == ports[j] && names[i] == names[j])
				throw(std::runtime_error("Error: Duplicate Servers."));
		}
	}
}

parser::Parser:: Parser(const char* input_filename)
{
	try
	{
		std::string error = input_filename;
		std::ifstream config_file(input_filename);
		if (!config_file)
			throw (std::runtime_error("Error: Failed to open [" + error + "]"));

		removeComments(config_file);
		storeTokens();
		evaluateTokens();
		splitIntoServerBlocks();
		createServerConf();
		checkDuplicates();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		exit(1);
	}
}
