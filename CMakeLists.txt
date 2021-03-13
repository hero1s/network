cmake_minimum_required(VERSION 3.15)
project(network)

set(CMAKE_CXX_STANDARD 14)

include_directories(public)
include_directories(src)

add_executable(network
        public/iocp_server.h
        public/memory_pools.h
        public/network_object.h
        src/acceptor.cpp
        src/acceptor.h
        src/circuit_queue.h
        src/connector.cpp
        src/connector.h
        src/double_list_t.h
        src/iocp_server.cpp
        src/IoHandler.cpp
        src/IoHandler.h
        src/memory_pools.cpp
        src/message.h
        src/network.h
        src/network_object.cpp
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
        src/tlock.h)
