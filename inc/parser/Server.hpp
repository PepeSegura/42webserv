/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: psegura- <psegura-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/12 19:08:22 by psegura-          #+#    #+#             */
/*   Updated: 2024/02/29 14:16:40 by psegura-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "Location.hpp"

typedef std::vector<std::vector<std::string> >		loc_blocks;

namespace parser {
	class Location;
	class Server {
		public:
			Server(std::vector<std::string> tokens);
			~Server();
			Server(const Server &f);
			Server& operator=(const Server &f);

			std::vector<parser::Location>	_locations;

			std::vector<parser::Location>						getLocations(void) const;
			std::pair <std::string, size_t>						getNetworkConf(void) const;
			std::string											getHost(void) const;
			size_t												getPort(void) const;
			std::string											getServerName(void) const;
			std::string											getRoot(void) const;
			size_t												getCMBS(void) const;
			bool												getAutoindex(void) const;
			std::vector<std::string>							getIndex(void) const;
			std::vector<std::string>							getAcceptMethod(void) const;
			std::vector<std::pair <std::string, std::string> >	getErrorPage(void) const;
			std::vector<std::pair <std::string, std::string> >	getCgiPass(void) const;

		private:
			Server();

			std::vector<std::string>		_tokens;
			loc_blocks						_location_blocks;

			std::pair <std::string, size_t>	_network_conf;
			std::string						_server_name;

			std::string						_root;
			size_t							_client_max_body_size;

			bool								_autoindex;
			std::vector<std::string>			_index;
			std::vector<std::string>			_accept_method;
			std::vector<std::pair <std::string, std::string> >	_error_page;
			std::vector<std::pair <std::string, std::string> >	_cgi_pass;

			void splitIntoLocationBlocks(void);
			void setServerConf(void);

			void chooseSetter(std::string level, size_t pos);

			void setListen(size_t i);
			void setServerName(size_t i);
			void setRoot(size_t i);
			void setAutoIndex(size_t i);
			void setIndexPages(size_t i);
			void setAcceptedMethods(size_t i);
			void setCMBS(size_t i);
			void setErrorPages(size_t i);
			void setCgiPass(size_t i);
	};
}

std::ostream&	operator<<(std::ostream& COUT, parser::Server& server);
