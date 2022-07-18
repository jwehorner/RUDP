/**
 * @file    ConnectionController.cpp
 * @author  James Horner
 * @brief   This file contains the definition of the RUDP ConnectionController.
 * @details The ConnectionController is responsible for creating, extending, and closing
 *          connections. The class is a singleton and includes mutexes to make it
 *          thread-safe.
 * @date 2022-06-30
 *
 * @copyright Copyright (c) 2022
 */
#ifndef CONNECTIONCONTROLLER_CPP
#define CONNECTIONCONTROLLER_CPP

#include "ConnectionController.hpp"

using namespace rudp;

ConnectionController *ConnectionController::instance = nullptr;

std::mutex ConnectionController::io_mutex;

std::map<int, Connection*> ConnectionController::connections;

int ConnectionController::connection_count = 0;

ConnectionController::ConnectionController() { }

ConnectionController *ConnectionController::getInstance()
{
    std::lock_guard<std::mutex> lock(io_mutex);
    if (instance == nullptr)
    {
        instance = new ConnectionController();
    }
    return instance;
}

int ConnectionController::addConnection()
{
    std::lock_guard<std::mutex> lock(io_mutex);
    ++connection_count;
    connections[connection_count] = new Connection(DEFAULT_TIMEOUT_MS);
    return connection_count;
}

int ConnectionController::addConnection(int timeout_ms)
{
    std::lock_guard<std::mutex> lock(io_mutex);
    ++connection_count;
    connections[connection_count] = new Connection(timeout_ms);
    return connection_count;
}

void ConnectionController::removeConnection(int connection_number)
{
    std::lock_guard<std::mutex> lock(io_mutex);
    connections.erase(connection_number);
}

rudp::Connection *ConnectionController::getConnection(int connection_number)
{
    std::lock_guard<std::mutex> lock(io_mutex);
    return connections.at(connection_number);
}

#endif /* CONNECTIONCONTROLLER_CPP */