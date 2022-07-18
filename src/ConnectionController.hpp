/**
 * @file ConnectionController.hpp
 * @author James Horner
 * @brief This file contains the declaration of the RUDP ConnectionController.
 * @details The ConnectionController is responsible for creating, extending, and closing 
 *          connections. The class is a singleton and includes mutexes to make it 
 *          thread-safe.
 * @date 2022-06-30
 * 
 * @copyright Copyright (c) 2022
 */
#ifndef CONNECTIONCONTROLLER_HPP
#define CONNECTIONCONTROLLER_HPP

#include <mutex>
#include <map>
#include <string>
#include <thread>

#include "rudp_macros.h"
#include "Connection.hpp"

namespace rudp
{
    /**
     * @brief   Class ConnectController is a Singleton that controls all RUDP connections for the library.
     * @details The singleton instance is responsible for creating, managing, removing, and supplying 
     *          access to connections. The class uses mutexes to ensure mutual exclusion between 
     *          client threads.
     */
    class ConnectionController
    {

    private:
        /// Instance of the ConnectionController Singleton.
        static ConnectionController *instance;
        /// Mutex to control access to the class.
        static std::mutex io_mutex;
        /// Map of connection numbers to Connection objects that the Singleton is managing.
        static std::map<int, Connection*> connections;
        /// Counter of connections that are used for the keys of the map.
        static int connection_count;

    protected:
        /**
         * @brief   Constructor for the ConnectionController class that is used by the 
         *          getInstance function.
         */
        ConnectionController();

    public:
        /**
         * @brief   Deleted cloning constructor so only one controller can exist.
         */
        ConnectionController(ConnectionController &other) = delete;

        /**
         * @brief   Deleted assignment operator so only one controller can exist.
         */
        void operator=(const ConnectionController &) = delete;

        /**
         * @brief   Get the instance of the ConnectionController.
         *
         * @return  ConnectionController* pointer to the ConnectionController.
         */
        static ConnectionController *getInstance();

        /**
         * @brief   Member to create a new Connection object and add it to the map of 
         *          active connections. 
         * @return  int connection number corresponding to the newly created connection.
         */
        static int addConnection();

        /**
         * @brief   Member to create a new Connection object and add it to the map of 
         *          active connections. 
         * @param   timeout_ms int for the length of the time to wait for an ACK before
         *          retransmission.
         * @return  int connection number corresponding to the newly created connection.
         */
        static int addConnection(int timeout_ms);

        /**
         * @brief   Member to remove a connection from the map of active connections.
         * @param   connection_number int number of the connection to be removed.
         */
        static void removeConnection(int connection_number);

        /**
         * @brief   Get an existing Connection object from the map of active connections.
         * 
         * @param   connection_number int number of the connection to be retrieved.
         * @return  Connection* pointer to connection object.
         */
        static Connection *getConnection(int connection_number);
    };
}

#endif /* CONNECTIONCONTROLLER_HPP */