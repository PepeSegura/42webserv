/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: psegura- <psegura-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/12 19:08:22 by psegura-          #+#    #+#             */
/*   Updated: 2024/02/28 20:45:01 by psegura-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "webserv.hpp"
#include "parser/Server.hpp"

namespace parser {
	class Server;
	class Location {
		public:
			Location(std::vector<std::string> tokens, parser::Server& server);
			Location();
			Location(const Location &f);
			~Location();
			Location& operator=(const Location &f);

			std::string											getURI(void) const;
			bool												getAutoindex(void) const;
			size_t												getCMBS(void) const;
			std::string											getRoot(void) const;
			std::string											getAlias(void) const;
			std::pair<std::size_t, std::string>					getReturn(void) const;
			std::vector<std::string>							getIndex(void) const;
			std::vector<std::string>							getAcceptMethod(void) const;
			std::string											getUploadStore(void) const;
			std::string											getUploadPass(void) const;
			std::vector<std::pair<std::string, std::string> >	getCgiPass(void) const;

		private:
			std::vector<std::string>			_tokens;

			std::string							_uri;
			bool								_autoindex;
			size_t								_client_max_body_size;

			std::string							_root;
			std::string							_alias;
			std::pair<std::size_t, std::string>	_return;

			std::vector<std::string>			_index;
			std::vector<std::string>			_accept_method;
			std::string							_upload_store;
			std::string							_upload_pass;

			std::vector<std::pair<std::string, std::string> > _cgi_pass;

			void inheritDataFromServer(parser::Server& server);
			
			void chooseSetter(std::string level, size_t pos);

			void setURI(size_t i);
			void setAutoIndex(size_t i);
			void setCMBS(size_t i);
			void setRoot(size_t i);
			void setAlias(size_t i);
			void setReturn(size_t i);
			void setIndexPages(size_t i);
			void setAcceptedMethods(size_t i);
			void setUploadStore(size_t i);
			void setUploadPass(size_t i);
			void setCgiPass(size_t i);
	};
}

std::ostream&	operator<<(std::ostream& COUT, parser::Location& location);
