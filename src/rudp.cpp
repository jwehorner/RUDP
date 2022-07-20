/**
 * @file 	rudp.h
 * @author 	James Horner (jwehorner@gmail.com)
 * @brief 	File rudp.h contains the defintion of the C interface of the RUDP library for sending and receiving packets.
 * @date 	2022-07-18
 * @copyright Copyright (c) 2022
 */
#ifndef RUDP_CPP
#define RUDP_CPP

#include <string>

#include "rudp.h"
#include "Connection.hpp"
#include "ConnectionController.hpp"

using namespace rudp;

int rudp_make_connection(int timeout_ms, int *error)
{
    try
    {
        int connection_number = ConnectionController::getInstance()->addConnection(timeout_ms);
        *error = 0;
        return connection_number;
    }
    catch (std::runtime_error runtime_error)
    {
        std::cout << runtime_error.what() << std::endl;
        *error = -1;
        return -1;
    }
}

void rudp_set_remote_endpoint(int connection, char *address, unsigned short port, int *error)
{
    try
    {
        ConnectionController::getInstance()->getConnection(connection)->setEndpointRemote(std::string(address), port);
        return;
    }
    catch (std::runtime_error runtime_error)
    {
        std::cout << runtime_error.what() << std::endl;
        *error = -1;
        return;
    }
}

void rudp_set_local_endpoint(int connection, unsigned short port, int *error)
{
    try
    {
        ConnectionController::getInstance()->getConnection(connection)->setEndpointLocal(port);
        return;
    }
    catch (std::runtime_error runtime_error)
    {
        std::cout << runtime_error.what() << std::endl;
        *error = -1;
        return;
    }
}

void rudp_reset_connection_send(int connection, int *error)
{
    try
    {
        ConnectionController::getInstance()->getConnection(connection)->resetConnectionSend();
        *error = 0;
        return;
    }
    catch (std::runtime_error runtime_error)
    {
        std::cout << runtime_error.what() << std::endl;
        *error = -1;
        return;
    }
}

void rudp_reset_connection_receive(int connection, int *error)
{
    try
    {
        ConnectionController::getInstance()->getConnection(connection)->resetConnectionReceive();
        *error = 0;
        return;
    }
    catch (std::runtime_error runtime_error)
    {
        std::cout << runtime_error.what() << std::endl;
        *error = -1;
        return;
    }
}

int rudp_send(int connection, const char *buf, int len, int *error)
{
    try
    {
        int sent_len = ConnectionController::getInstance()->getConnection(connection)->send(buf, len);
        *error = 0;
        return sent_len;
    }
    catch (std::runtime_error runtime_error)
    {
        std::cout << runtime_error.what() << std::endl;
        *error = -1;
        return -1;
    }
}

int rudp_receive(int connection, char *buf, int len, char *address_remote, int *port_remote, int *error)
{
    try
    {
        int received_len = ConnectionController::getInstance()->getConnection(connection)->receive(buf, len, address_remote, port_remote);
        *error = 0;
        return received_len;
    }
    catch (std::runtime_error runtime_error)
    {
        std::cout << runtime_error.what() << std::endl;
        *error = -1;
        return -1;
    }
}

#endif /* RUDP_CPP */