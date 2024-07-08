/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: psegura- <psegura-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/12 19:08:22 by psegura-          #+#    #+#             */
/*   Updated: 2024/02/08 19:08:45 by psegura-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "Server.hpp"

typedef std::vector<parser::Server> Servers;

typedef std::vector<std::vector<std::string> > sv_blocks;

namespace parser {
	class Parser
	{
		public:
			Parser(const char* input_filename);
			~Parser();

			std::vector<parser::Server> getServers(void);

		private:
			std::string					_config_file;
			std::vector<std::string>	_tokens;
			std::vector<parser::Server> _servers;
			sv_blocks					_server_blocks;

			void	removeComments(std::ifstream& config_file);
			void	storeTokens(void);
			void	evaluateTokens(void);
			void	splitIntoServerBlocks(void);
			
			void	createServerConf(void);
			void	checkDuplicates(void);
	};
}

std::ostream&	operator<<(std::ostream& COUT, parser::Parser& parser);
