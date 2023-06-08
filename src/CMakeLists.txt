set(SOCKET_SRC socket/socket.cpp socket/socket.hpp)

add_executable(${PROJECT_NAME} main.cpp ${SOCKET_SRC})