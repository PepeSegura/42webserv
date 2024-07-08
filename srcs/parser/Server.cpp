/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: psegura- <psegura-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/12 19:23:08 by psegura-          #+#    #+#             */
/*   Updated: 2024/02/29 14:23:39 by psegura-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parser/Server.hpp"
#include "Server.hpp"

#define B 1
#define KB 1000
#define MB (KB * 1000)
#define DEFAULT_PORT 80

void removeBrackets(std::string& str)
{
    if (!str.empty() && str[0] == '{' && str[str.size() - 1] == '}') {
        str.erase(0, 1);
        str.erase(str.size() - 1);
    }
}

static bool isOnlyDigits(const std::string& str)
{
	for (size_t i = 0; i < str.length(); i++) {
		if (!std::isdigit(str[i]))
			return (false);
    }
    return (true);
}

static std::vector<std::string> split(const std::string& input, char delimiter)
{
    std::vector<std::string> tokens;
    std::stringstream ss(input);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }	
    return (tokens);
}

void parser::Server:: splitIntoLocationBlocks(void)
{
    std::vector<std::string> currentLocation;
	std::vector<std::string> aux;
    bool inLocationBlock = false;

    for (size_t i = 0; i < _tokens.size(); i++) {
        if (_tokens[i] == "location")
            inLocationBlock = true;

        if (inLocationBlock)
		{
            currentLocation.push_back(_tokens[i]);
            if (_tokens[i].find('{') != std::string::npos)
                inLocationBlock = true;
            if (_tokens[i].find('}') != std::string::npos)
			{
                inLocationBlock = false;
                removeBrackets(currentLocation.back());
                _location_blocks.push_back(currentLocation);
                currentLocation.clear();
            }
        } else
            aux.push_back(_tokens[i]);
    }
	_tokens.clear();
	_tokens = aux;
}

static int	countParameters(std::vector<std::string>& tokens, size_t i)
{
	int	count = 0;
	for (++i; i < tokens.size() && tokens[i] != ";"; i++)
		count++;	
	return (count);
}

static bool validateOctects(std::vector<std::string>& octecs)
{
	if (octecs.size() != 4)
		return (false);
	for (size_t i = 0; i < octecs.size(); i++) {
		if (octecs[i].size() > 3 || octecs[i].size() == 0 || isOnlyDigits(octecs[i]) == false)
			return (false);
	}
	for (size_t i = 0; i < octecs.size(); i++) {
		int	nbr = atoi(octecs[i].c_str());
		if (nbr > 255 || nbr < 0)
			return (false);
	}
	return (true);
}

static bool parseAddress(std::string& token)
{
	std::vector<std::string> octecs;

	if (token == "localhost")
		return (true);
	octecs = split(token, '.');
	if (validateOctects(octecs) == false)
		return (false);
	return (true);
}

enum cases{
	ERROR,
	ONLY_ADDRESS,
	ONLY_PORT,
	BOTH,
};

static int parseIP(std::vector<std::string> tokens)
{
	if (tokens.size() == 1 && (isOnlyDigits(tokens[0]) == true && tokens[0].size() < 6))
		return (ONLY_PORT);
	if (tokens.size() == 1 && parseAddress(tokens[0]) == true)
		return (ONLY_ADDRESS);
	if (tokens.size() == 2 && (parseAddress(tokens[0]) == true && (isOnlyDigits(tokens[1]) == true && tokens[1].size() < 6)))
		return (BOTH);
	return (ERROR);
}
static bool countSeparators(std::string& input)
{
	int	dots = 0;
	int colon = 0;

	for (size_t i = 0; i < input.size(); i++) {
		if (input[i] == '.')
			dots++;
		if (input[i] == ':')
			colon++;
	}
	if (dots > 3 || colon > 1)
		return (false);
	return (true);
}

void parser::Server:: setListen(size_t i)
{
	if (countParameters(_tokens, i) != 1 || countSeparators(_tokens[i + 1]) == false)
		throw (std::runtime_error("Error in the configuration file in the rule: [" + _tokens[i] + "]"));

	std::vector<std::string> splitted = split(_tokens[i + 1], ':');
	int	mode = parseIP(splitted);
	if (mode == ERROR)
		throw (std::runtime_error("Error in the configuration file in the rule: [" + _tokens[i] + "]"));

	if (splitted[0] == "localhost")
		splitted[0] = "127.0.0.1";
	if (mode == ONLY_ADDRESS)
		_network_conf = std::make_pair(splitted[0], DEFAULT_PORT);
	if (mode == ONLY_PORT)
		_network_conf = std::make_pair("0", atoi(splitted[0].c_str()));
	if (mode == BOTH)
		_network_conf = std::make_pair(splitted[0], atoi(splitted[1].c_str()));
	if (_network_conf.second > 65535)
		throw (std::runtime_error("Error in the configuration file in the rule: [" + _tokens[i] + "] port is out of range."));
}

void parser::Server:: setServerName(size_t i)
{
	if (countParameters(_tokens, i) != 1)
		throw (std::runtime_error("Error in the configuration file in the rule: [" + _tokens[i] + "]"));
	_server_name = _tokens[i + 1];
}

static bool isAbsolutePath(std::string& string)
{
	return (string[0] == '/');
}

void parser::Server:: setRoot(size_t i)
{
	if (countParameters(_tokens, i) != 1)
		throw (std::runtime_error("Error in the configuration file in the rule: [" + _tokens[i] + "]"));
	if (!isAbsolutePath(_tokens[i + 1]) == true)
		throw (std::runtime_error("Error in the configuration file in the rule: [" + _tokens[i] + "] " + "with relative path [" + _tokens[i + 1] + "]"));
	_root = _tokens[i + 1];
}

static int getState(int x, int y)
{
	const int	states[][5] = {
		{1, 2, 1, 1}, // 0 -> START
		{1, 1, 1, 1}, // 1 -> ERROR
		{1, 2, 3, 4}, // 2 -> DIGIT
		{1, 1, 1, 1}, // 3 -> K
		{1, 1, 1, 1}, // 4 -> M
	};
	return (states[x][y]);
}
const std::string name_states[] = {
	"START",
	"ERROR",
	"DIGIT",
	"K",
	"M",
};

static int chooseState(int state, const char letter)
{
	int pos = 0;
	if (std::isdigit(letter) == 1)
		pos = 1;
	else if (std::toupper(letter) == 'K')
		pos = 2;
	else if (std::toupper(letter) == 'M')
		pos = 3;
	state = getState(state, pos);
	// std::cerr << "POS: " << pos << " State: [" << name_states[state] << "] Token: [" << letter << "]" << std::endl;
	return (state);
}

void parser::Server:: setCMBS(size_t i)
{
	int state = 0;
	for (size_t j = 0; j < _tokens[i + 1].length(); j++)
		state = chooseState(state, _tokens[i + 1][j]);
	if (countParameters(_tokens, i) != 1 || state < 2)
		throw (std::runtime_error("Error in the configuration file in the rule: [" + _tokens[i] + "]"));
	int size = 0;
	switch (state)
	{
		case 2:		size = B;	break;
		case 3:		size = KB;	break;
		case 4:		size = MB;	break;
		default:	size = B;	break;
	}
	_client_max_body_size = atoi(_tokens[i + 1].c_str()) * size;
}

void parser::Server:: setIndexPages(size_t i)
{
	if (countParameters(_tokens, i) < 1)
		throw (std::runtime_error("Error in the configuration file in the rule: [" + _tokens[i] + "]"));
	for (++i; i < _tokens.size() && _tokens[i] != ";"; i++)
		_index.push_back(_tokens[i]);
}

void parser::Server:: setAcceptedMethods(size_t i)
{
	if (countParameters(_tokens, i) < 1)
		throw (std::runtime_error("Error in the configuration file in the rule: [" + _tokens[i] + "]"));
	size_t	error = i;
	for (++i; i < _tokens.size() && _tokens[i] != ";"; i++)
	{
		if (_tokens[i] == "GET" || _tokens[i] == "POST" || _tokens[i] == "DELETE")
			_accept_method.push_back(_tokens[i]);
		else
			throw (std::runtime_error("Error in the configuration file in the rule: [" + _tokens[error] + "]"));
	}
}

void parser::Server:: setAutoIndex(size_t i)
{
	if (countParameters(_tokens, i) != 1)
		throw (std::runtime_error("Error in the configuration file in the rule: [" + _tokens[i] + "]"));
	if (_tokens[i + 1] == "on" || _tokens[i + 1] == "true" || _tokens[i + 1] == "1")
		_autoindex = 1;
	else if (_tokens[i + 1] == "off" || _tokens[i + 1] == "false" || _tokens[i + 1] == "0")
		_autoindex = 0;
	else
		throw (std::runtime_error("Error in the configuration file in the rule: [" + _tokens[i] + "]"));
}

void parser::Server:: setErrorPages(size_t i)
{
	if (countParameters(_tokens, i) != 2)
		throw (std::runtime_error("Error in the configuration file in the rule: [" + _tokens[i] + "]"));
	if (isOnlyDigits(_tokens[i + 1]) == false)
		throw (std::runtime_error("Error in the configuration file in the rule: [" + _tokens[i] + "]"));
	_error_page.push_back(std::make_pair(_tokens[i + 1], _tokens[i + 2]));
}

void parser::Server:: setCgiPass(size_t i)
{
	if (countParameters(_tokens, i) < 2)
		throw (std::runtime_error("Error in the configuration file in the rule: [" + _tokens[i] + "]"));
	std::string last;

	for (size_t j = i; j < _tokens.size(); j++) {
		if (_tokens[j] == ";")
		{
			last = _tokens[j - 1];
			break;
		}
	}
	for (++i ; i < _tokens.size() && _tokens[i] != last; i++) {
		_cgi_pass.push_back(std::make_pair(_tokens[i], last));
	}
}


void parser::Server:: setServerConf(void)
{
	for (size_t i = 0; i < _tokens.size(); i++)
		chooseSetter(_tokens[i], i);
}

void parser::Server:: chooseSetter(std:: string level, size_t pos)
{
	std::string keywords[9] =
	{
		"root",
		"client_max_body_size",
		"index",
		"autoindex",
		"accept_method",
		"listen",
		"server_name",
		"cgi_pass",
		"error_page",
	};

	void	(parser::Server:: *setters[9])(size_t i) = 
	{
		&Server::setRoot			,
		&Server::setCMBS			,
		&Server::setIndexPages		,
		&Server::setAutoIndex		,
		&Server::setAcceptedMethods	,
		&Server::setListen			,
		&Server::setServerName		,
		&Server::setCgiPass			,
		&Server::setErrorPages		,
	};

	for (size_t i = 0; i < 9; i++)
	{
		if (level == keywords[i])
		{
			(this->*setters[i])(pos);
			return ;
		}
	}
	return ;
}

std::ostream&	operator<<(std::ostream& COUT, parser::Server& server)
{
	std::pair<std::string, size_t>						_network_conf = server.getNetworkConf();
	std::string											_server_name = server.getServerName();
	std::string											_root = server.getRoot();
	size_t 												_client_max_body_size = server.getCMBS();
	bool												_autoindex = server.getAutoindex();
	std::vector<std::string>							_index = server.getIndex();
	std::vector<std::string>							_accept_method = server.getAcceptMethod();
	std::vector<std::pair<std::string, std::string> >	_error_page = server.getErrorPage();
	std::vector<std::pair<std::string, std::string> >	_cgi_pass = server.getCgiPass();
	std::vector<parser::Location>						_locations = server.getLocations();


	COUT << "       SERVER        "	<< std::endl;
	COUT << "---------------------"	<< std::endl;

	COUT << "Listen:          [" << _network_conf.first			<< "] [" << _network_conf.second << "]"<< std::endl;
	COUT << "Server name:     [" << _server_name				<< "]" << std::endl;
	if (!_root.empty())
		COUT << "Root:            [" << _root					<< "]" << std::endl;
	COUT << "Autoindex:       [" 	 << _autoindex				<< "]" << std::endl;
	COUT << "ClientMBS:       ["	 << _client_max_body_size 	<< "]" << std::endl;

	if (_index.size() != 0)
	{
		COUT << "Index:          ";
		for (size_t i = 0; i < _index.size(); i++)
			COUT << " [" << _index[i] << "]";
		COUT << std::endl;
	}

	if (_accept_method.size() != 0)
	{	
		COUT << "Acepted Methods:";
		for (size_t i = 0; i < _accept_method.size(); i++)
			COUT << " [" << _accept_method[i] << "]";
		COUT << std::endl;
	}

	if (_error_page.size() != 0)
	{
		COUT << "Error pages:\n";
		for (size_t i = 0; i < _error_page.size(); i++)
			COUT << "\t[" << _error_page[i].first << "] -> [" << _error_page[i].second << "]" << std::endl;
		COUT << std::endl;
	}

	if (!_cgi_pass.empty())
	{
		COUT << "Cgi Pass:" << std::endl;
		for (size_t i = 0; i < _cgi_pass.size(); i++)
			COUT << "    [" << _cgi_pass[i].first << "] -> [" << _cgi_pass[i].second << "]" << std::endl;
	}

	COUT << std::endl;
	for (size_t i = 0; i < _locations.size(); i++)
		COUT << _locations[i];
	return (COUT);
}

std::vector<parser::Location> parser::Server:: getLocations(void) const
{
	return (_locations);
}

std::pair <std::string, size_t> parser::Server:: getNetworkConf(void) const
{
	return (_network_conf);
}

std::string parser::Server::getHost(void) const
{
    return (_network_conf.first);
}

size_t parser::Server::getPort(void) const
{
	return (_network_conf.second);
}

std::string parser::Server::getServerName(void) const
{
	return (_server_name);
}

std::string parser::Server:: getRoot(void) const
{
	return (_root);
}

size_t parser::Server:: getCMBS(void) const
{
	return (_client_max_body_size);
}

std::vector<std::string> parser::Server:: getIndex(void) const		
{
	return (_index); 
}

bool parser::Server:: getAutoindex(void) const	
{
	return (_autoindex); 
}

std::vector<std::string> parser::Server:: getAcceptMethod(void) const
{
	return (_accept_method); 
}

std::vector<std::pair <std::string, std::string> > parser::Server:: getErrorPage(void) const
{
	return (_error_page);
}

std::vector<std::pair <std::string, std::string> > parser::Server:: getCgiPass(void) const
{
	return (_cgi_pass);
}


parser::Server:: Server() {}

parser::Server:: ~Server() {}

parser::Server:: Server(const Server &f)
{
	this->_locations			= f._locations;

	this->_tokens				= f._tokens;
	this->_location_blocks		= f._location_blocks;

	this->_network_conf			= f._network_conf;
	this->_server_name			= f._server_name;

	this->_root 				= f._root;
	this->_client_max_body_size = f._client_max_body_size;
	this->_index				= f._index;
	this->_autoindex			= f._autoindex;

	this->_cgi_pass				= f._cgi_pass;
	this->_error_page			= f._error_page;

	this->_accept_method		= f._accept_method;
}

parser::Server& parser::Server:: operator=(const Server& f)
{
    if (this != &f)
		*this = f;
    return (*this);
}

parser::Server:: Server(std::vector<std::string> tokens)
{
	_tokens = tokens;
	_network_conf.first = "0";
	_network_conf.second = DEFAULT_PORT;
	_client_max_body_size = 10000000;
	_autoindex = false;
	splitIntoLocationBlocks();
	setServerConf();

	for (size_t i = 0; i < _location_blocks.size(); i++)
		_locations.push_back(parser::Location(_location_blocks[i], *this));
}
