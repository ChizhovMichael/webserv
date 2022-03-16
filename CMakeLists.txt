cmake_minimum_required(VERSION 3.19)
project(webserv)

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

include_directories(./srcs)

add_compile_options(-Wall -Wextra -Werror)

add_executable(
        webserv

        srcs/main.cpp
        srcs/Config.cpp srcs/Config.hpp
        srcs/Server.cpp srcs/Server.hpp
        srcs/Location.cpp srcs/Location.hpp
        srcs/Daemon.cpp srcs/Daemon.hpp
        srcs/Events.cpp srcs/Events.hpp
        srcs/IEventSubscriber.hpp
        srcs/Socket.cpp srcs/Socket.hpp
        srcs/Connection.cpp srcs/Connection.hpp
        srcs/AHttpMessage.cpp srcs/AHttpMessage.hpp
        srcs/HttpRequest.cpp srcs/HttpRequest.hpp
        srcs/HttpResponse.cpp srcs/HttpResponse.hpp
        srcs/Cgi.cpp srcs/Cgi.hpp
        srcs/utils/Print.cpp srcs/utils/Print.hpp
        srcs/utils/Path.cpp srcs/utils/Path.hpp
        srcs/utils/MimeType.cpp srcs/utils/MimeType.hpp
)
