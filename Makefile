SRC_FILES	= main.cpp \
				Config.cpp \
				Location.cpp \
				Server.cpp \
				Socket.cpp \
				Connection.cpp \
				AHttpMessage.cpp \
				HttpRequest.cpp \
				HttpResponse.cpp \
				Events.cpp \
				Daemon.cpp \
				Cgi.cpp \
				utils/Path.cpp \
				utils/FileStats.cpp \
				utils/Print.cpp \
				utils/MimeType.cpp

NAME	= webserv

CC		= clang++ -O3
RM		= rm -f

CFLAGS	= -Wall -Wextra -Werror -std=c++98 -pedantic

SRCS_DIR = ./srcs
OBJS_DIR = ./objs

SRCS	= $(addprefix $(SRCS_DIR)/, $(SRC_FILES))
OBJS	= $(patsubst $(SRCS_DIR)/%.cpp,$(OBJS_DIR)/%.o, $(SRCS))
DEPS	= $(OBJS:.o=.d)

all:		$(NAME)

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.cpp | $(OBJS_DIR)
			$(CC) $(CFLAGS) -I$(SRCS_DIR) -MMD -MP -c -o $@ $<

$(OBJS_DIR):
			mkdir -p $(OBJS_DIR)

$(NAME):	$(OBJS)
			$(CC) -g $(CFLAGS) -I$(SRCS_DIR) -MMD -MP -o $(NAME) $(OBJS)

clean:
			@rm -rf $(OBJS)
			@rm -rf $(DEPS)

fclean:		clean
			@rm $(NAME)

re:			fclean all

.PHONY:		all clean fclean re
