NAME := webserv

CXX := c++ -std=c++98
CXXFLAGS := -Wall -Wextra -Werror
DEBUG := -g3 -fsanitize=address

RM := rm -rf

BUILD_DIR := .build/
SRC_DIR := srcs/
INC_DIR := inc/

NET_DIR := network/
CGI_DIR := cgi/
PARSER_DIR := parser/
REQ_DIR := request/

NET_SRCS := $(addprefix $(NET_DIR), servermanager connections socket serversocket clientsocket consolelog deletefile)
CGI_SRCS := $(addprefix $(CGI_DIR),  cgimanager)
PARSER_SRCS := $(addprefix $(PARSER_DIR), Location Parser Server trim)
REQ_SRCS := $(addprefix $(REQ_DIR), request parse_tools errors response_utils)

PRESRCS := main $(NET_SRCS) $(CGI_SRCS) $(PARSER_SRCS) $(REQ_SRCS)

SRCS :=  $(addprefix $(SRC_DIR), $(addsuffix .cpp, $(PRESRCS)))
OBJS := $(SRCS:%=$(BUILD_DIR)%.o)
DEPS := $(OBJS:.o=.d)

INC := $(INC_DIR) $(addprefix $(INC_DIR), $(NET_DIR) $(CGI_DIR) $(PARSER_DIR) $(REQ_DIR))
INC_FLAGS := $(addprefix -I, $(INC))

CPPFLAGS := $(INC_FLAGS) -MMD -MP

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(OBJS) $(DEBUG) -o $@

$(BUILD_DIR)%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEBUG) -c $< -o $@

clean:
	$(RM) $(BUILD_DIR)

fclean: clean
	$(RM) $(NAME)

re:: fclean
re:: $(NAME)

valgrind: $(NAME)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --track-fds=yes ./$(NAME)

cgitest:
	$(CXX) $(INC_FLAGS) $(DEBUG) -o cgitest cgi_test/main.cpp srcs/cgi/cgimanager.cpp srcs/network/consolelog.cpp

clean_cgi_test:
	$(RM) cgitest

.PHONY: all clean fclean re valgrind

-include $(DEPS)
