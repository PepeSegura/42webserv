/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: psegura- <psegura-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/12 19:23:08 by psegura-          #+#    #+#             */
/*   Updated: 2024/03/12 18:58:02 by psegura-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parser/Location.hpp"

#define B 1
#define KB 1000
#define MB (KB * 1000)

static bool isOnlyDigits(const std::string& str) {
	for (size_t i = 0; i < str.length(); i++) {
		if (!std::isdigit(str[i]))
			return (false);
	}
	return (true);
}

static int	countParameters(std::vector<std::string>& tokens, size_t i)
{
	int	count = 0;
	for (++i; i < tokens.size() && tokens[i] != ";"; i++)
		count++;
	return (count);
}

static bool isAbsolutePath(std::string& string)
{
	return (string[0] == '/');
}

void parser::Location:: setURI(size_t i)
{
	if (!isAbsolutePath(_tokens[i + 1]) == true)
		throw (std::runtime_error("Error in the configuration file in the rule: [" + _tokens[i] + "] " + "with relative path [" + _tokens[i + 1] + "]"));
	_uri = _tokens[i + 1];
}

void parser::Location:: setAutoIndex(size_t i)
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

static int chooseStateL(int state, const char letter)
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

void parser::Location:: setCMBS(size_t i)
{
	int state = 0;
	for (size_t j = 0; j < _tokens[i + 1].length(); j++)
		state = chooseStateL(state, _tokens[i + 1][j]);
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

void parser::Location:: setRoot(size_t i)
{
	if (countParameters(_tokens, i) != 1)
		throw (std::runtime_error("Error in the configuration file in the rule: [" + _tokens[i] + "]"));
	if (!isAbsolutePath(_tokens[i + 1]) == true)
		throw (std::runtime_error("Error in the configuration file in the rule: [" + _tokens[i] + "] " + "with relative path [" + _tokens[i + 1] + "]"));
	_root = _tokens[i + 1];
}

void parser::Location:: setAlias(size_t i)
{
	if (countParameters(_tokens, i) != 1)
		throw (std::runtime_error("Error in the configuration file in the rule: [" + _tokens[i] + "]"));
	if (!isAbsolutePath(_tokens[i + 1]) == true)
		throw (std::runtime_error("Error in the configuration file in the rule: [" + _tokens[i] + "] " + "with relative path [" + _tokens[i + 1] + "]"));
	_alias = _tokens[i + 1];
}

void parser::Location:: setReturn(size_t i)
{
	if (countParameters(_tokens, i) != 2 || isOnlyDigits(_tokens[i + 1]) == false)
		throw (std::runtime_error("Error in the configuration file in the rule: [" + _tokens[i] + "]"));
	_return = std::make_pair(atoi(_tokens[i + 1].c_str()), _tokens[i + 2]);
}

void parser::Location:: setIndexPages(size_t i)
{
	if (countParameters(_tokens, i) < 1)
		throw (std::runtime_error("Error in the configuration file in the rule: [" + _tokens[i] + "]"));
	_index.clear();
	for (++i; i < _tokens.size() && _tokens[i] != ";"; i++)
		_index.push_back(_tokens[i]);
}

void parser::Location:: setAcceptedMethods(size_t i)
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

void parser::Location:: setUploadStore(size_t i)
{
	if (countParameters(_tokens, i) != 1)
		throw (std::runtime_error("Error in the configuration file in the rule: [" + _tokens[i] + "]"));
	if (!isAbsolutePath(_tokens[i + 1]) == true)
		throw (std::runtime_error("Error in the configuration file in the rule: [" + _tokens[i] + "] " + "with relative path [" + _tokens[i + 1] + "]"));
	_upload_store = _tokens[i + 1];
}

void parser::Location:: setUploadPass(size_t i)
{
	if (countParameters(_tokens, i) != 1)
		throw (std::runtime_error("Error in the configuration file in the rule: [" + _tokens[i] + "]"));
	if (!isAbsolutePath(_tokens[i + 1]) == true)
		throw (std::runtime_error("Error in the configuration file in the rule: [" + _tokens[i] + "] " + "with relative path [" + _tokens[i + 1] + "]"));
	_upload_pass = _tokens[i + 1];
}

void parser::Location:: setCgiPass(size_t i)
{
	if (countParameters(_tokens, i) != 2)
		throw (std::runtime_error("Error in the configuration file in the rule: [" + _tokens[i] + "]"));
	_cgi_pass.push_back(std::make_pair(_tokens[i + 1], _tokens[i + 2]));
}


void parser::Location:: chooseSetter(std:: string level, size_t pos)
{
	std::string keywords[11] =
	{
		"location",
		"autoindex",
		"client_max_body_size",
		"root",
		"alias",
		"return",
		"index",
		"accept_method",
		"upload_store",
		"upload_pass",
		"cgi_pass",
	};

	void	(parser::Location:: *setters[11])(size_t i) =
	{
		&Location::setURI,
		&Location::setAutoIndex,
		&Location::setCMBS,
		&Location::setRoot,
		&Location::setAlias,
		&Location::setReturn,
		&Location::setIndexPages,
		&Location::setAcceptedMethods,
		&Location::setUploadStore,
		&Location::setUploadPass,
		&Location::setCgiPass,
	};

	for (size_t i = 0; i < 11; i++)
	{
		if (level == keywords[i])
		{
			(this->*setters[i])(pos);
			return ;
		}
	}
	return ;
}

std::ostream&	operator<<(std::ostream& COUT, parser::Location& location)
{
	std::string							_uri = location.getURI();
	bool								_autoindex = location.getAutoindex();
	size_t								_client_max_body_size = location.getCMBS();
	std::string							_root = location.getRoot();
	std::string							_alias = location.getAlias();
	std::pair<std::size_t, std::string>	_return = location.getReturn();
	std::vector<std::string>			_index = location.getIndex();
	std::vector<std::string>			_accept_method = location.getAcceptMethod();
	std::string							_upload_store = location.getUploadStore();
	std::string							_upload_pass = location.getUploadPass();
	std::vector<std::pair<std::string, std::string> > _cgi_pass = location.getCgiPass();

	COUT << "       LOCATION      "	<< std::endl;
	COUT << "---------------------"	<< std::endl;

	COUT << "URI:             [" 		<< _uri						<< "]" << std::endl;
	COUT << "Autoindex:       [" 		<< _autoindex				<< "]" << std::endl;
	COUT << "ClientMBS:       [" 		<< _client_max_body_size	<< "]" << std::endl;
	if (!_root.empty())
		COUT << "Root:            [" 	<< _root 					<< "]" << std::endl;
	if (!_alias.empty())
		COUT << "Alias:           [" 	<< _alias 					<< "]" << std::endl;

	if (_return.first != 0 && !_return.second.empty())
	{
		COUT << "Return:"	<< std::endl;
		COUT << "  Response:      ["	<< _return.first 	<< "]" << std::endl;
		COUT << "  Redirect:      ["	<< _return.second	<< "]" << std::endl;
	}

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

	if (!_upload_store.empty())
		COUT << "Upload store:    [" << _upload_store 					<< "]" << std::endl;
	if (!_upload_pass.empty())
		COUT << "Upload pass:     [" << _upload_pass					<< "]" << std::endl;
	if (!_cgi_pass.empty())
	{
		COUT << "Cgi Pass:" << std::endl;
		for (size_t i = 0; i < _cgi_pass.size(); i++)
			COUT << "    [" << _cgi_pass[i].first << "] -> [" << _cgi_pass[i].second << "]" << std::endl;
	}
	COUT << "---------------------" << std::endl;
	COUT << std::endl;
	return (COUT);
}

std::string parser::Location:: getURI(void) const
{
	return (_uri);
}

bool parser::Location:: getAutoindex(void) const
{
	return (_autoindex);
}

size_t parser::Location:: getCMBS(void) const
{
	return (_client_max_body_size);
}

std::string parser::Location:: getRoot(void) const
{
	return (_root);
}

std::string parser::Location:: getAlias(void) const
{
	return (_alias);
}

std::pair<std::size_t, std::string> parser::Location:: getReturn(void) const
{
	return (_return);
}

std::vector<std::string> parser::Location:: getIndex(void) const
{
	return (_index);
}

std::vector<std::string> parser::Location:: getAcceptMethod(void) const
{
	return (_accept_method);
}

std::string parser::Location:: getUploadStore(void) const
{
	return (_upload_store);
}

std::string parser::Location:: getUploadPass(void) const
{
	return (_upload_pass);
}

std::vector<std::pair<std::string, std::string> > parser::Location:: getCgiPass(void) const
{
	return (_cgi_pass);
}


parser::Location:: Location() {}

parser::Location:: ~Location() {}

parser::Location:: Location(const Location& f)
{
	this->_uri                  = f._uri;
	this->_autoindex			= f._autoindex;
	this->_client_max_body_size = f._client_max_body_size;

	this->_root				    = f._root;
	this->_alias			    = f._alias;
	this->_return		        = f._return;

	this->_index			    = f._index;
	this->_accept_method		= f._accept_method;
	this->_upload_store		    = f._upload_store;
	this->_upload_pass		    = f._upload_pass;

	this->_cgi_pass				= f._cgi_pass;
}

parser::Location& parser::Location:: operator=(const Location& f)
{
	if (this != &f)
	{
		this->_uri                  = f._uri;
		this->_autoindex			= f._autoindex;
		this->_client_max_body_size = f._client_max_body_size;
		this->_root				    = f._root;
		this->_alias			    = f._alias;
		this->_return		        = f._return;
		this->_index			    = f._index;
		this->_accept_method		= f._accept_method;
		this->_upload_store		    = f._upload_store;
		this->_upload_pass		    = f._upload_pass;
		this->_cgi_pass				= f._cgi_pass;
	}
	return (*this);
}

void  parser::Location:: inheritDataFromServer(parser::Server& server)
{
	_root = server.getRoot();
	_client_max_body_size = server.getCMBS();
	_index = server.getIndex();
	_autoindex = server.getAutoindex();
	_cgi_pass = server.getCgiPass();
	_accept_method = server.getAcceptMethod();
}

parser::Location:: Location(std::vector<std::string> tokens, parser::Server& server)
{
	inheritDataFromServer(server);
	_tokens = tokens;
	for (size_t i = 0; i < _tokens.size(); i++)
		chooseSetter(_tokens[i], i);
}
