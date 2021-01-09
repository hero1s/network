cmake_minimum_required(VERSION 3.9)
project(network)

set(CMAKE_CXX_STANDARD 11)

add_executable(network
        src/acceptor.cpp
        src/acceptor.h
        src/circuit_queue.h
        src/connector.cpp
        src/connector.h
        src/double_list_t.h
        src/iocp_server.cpp
        src/iocp_server.h
        src/IoHandler.cpp
        src/IoHandler.h
        src/network.h
        src/network_object.cpp
        src/network_object.h
        src/recv_buffer.h
        src/send_buffer.h
        src/session.cpp
        src/session.h
        src/session_list.cpp
        src/session_list.h
        src/session_pool.cpp
        src/session_pool.h
        src/single_list_t.h
        src/socket_ops.cpp
        src/socket_ops.h
        src/tlock.cpp
        src/tlock.h src/message.h)
