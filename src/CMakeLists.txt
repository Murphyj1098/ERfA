set(SOCKET_SRC socket.cpp socket.hpp)
set(RELAY_SRC relay.cpp relay.hpp)

add_executable(${PROJECT_NAME} main.cpp types.hpp ${SOCKET_SRC} ${RELAY_SRC})
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads)
target_link_libraries(${PROJECT_NAME} Threads::Threads)
install(TARGETS ${PROJECT_NAME} DESTINATION bin)