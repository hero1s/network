cmake_minimum_required(VERSION 3.9)
project(network)

set(CMAKE_CXX_STANDARD 11)

add_executable(network
        src/Acceptor.cpp
        src/Acceptor.h
        src/CircuitQueue.h
        src/Connector.cpp
        src/Connector.h
        src/Double_List_T.h
        src/IOCPServer.cpp
        src/IOCPServer.h
        src/IoHandler.cpp
        src/IoHandler.h
        src/Network.h
        src/NetworkObject.cpp
        src/NetworkObject.h
        src/RecvBuffer.h
        src/SendBuffer.h
        src/Session.cpp
        src/Session.h
        src/SessionList.cpp
        src/SessionList.h
        src/SessionPool.cpp
        src/SessionPool.h
        src/Single_List_T.h
        src/SocketOps.cpp
        src/SocketOpt.h
        src/TLock.cpp
        src/TLock.h src/Message.h)
