/**
 * @file 	Connection.hpp
 * @author 	James Horner (jwehorner@gmail.com)
 * @brief 	File Connection.cpp contains the declaration of the Connection class of the RUDP library.
 * @details The Connection class represents a virtual connection between two RUDP connection objects
 * 			based on their sequence numbers. When sending a packet using a connection, the packet will
 * 			be retransmitted until an ACK with the correct sequence is received. If a packet is
 * 			received by a connection, an ACK will be sent in responce but the function will not
 * 			return until a message with the correct sequence is received.
 * @date 	2022-07-18
 * @copyright Copyright (c) 2022
 */
#ifndef CONNECTION_HPP
#define CONNECTION_HPP

// Standard Libraries
#include <array>
#include <chrono>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <vector>

// Boost networking libraries
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

// Library macros header
#include "rudp_macros.h"

namespace rudp
{
    /**
     * @brief   Class Connection represents a virtual connection over which UDP packets can be sent.
     * @details The Connection class uses a sequence number and the Stop-and-Wait ARQ protocol to
     *          send data between two Connection objects.
     */
    class Connection
    {
    private:
        /// Mutex for thread synchronization.
        std::mutex io_mutex;
        /// Timeout after which the connection will retransmit a message.
        int timeout_ms;
        /// Sequence number of the message that the connection is currently sending.
        uint16_t sequence_send;
        /// Map of senders to their receive sequence numbers.
        std::map<std::string, uint16_t> sequence_recv_map;

        /// Boost IO service for networking.
        boost::asio::io_service io_service;
        /// Socket over which packets will be sent/received.
        boost::asio::ip::udp::socket socket{io_service};
        /// Local endpoint where packets will be received.
        boost::asio::ip::udp::endpoint endpoint_local;
        /// Remote endpoint where packets will be sent.
        boost::asio::ip::udp::endpoint endpoint_remote;
        /// Flag for if the local endpoint has been set.
        bool has_endpoint_local;
        /// Flag for if the remote endpoint has been set.
        bool has_endpoint_remote;

        boost::asio::deadline_timer timer{io_service};

        void check_deadline();

        void handle_receive(const boost::system::error_code &err, std::size_t length, boost::system::error_code *err_out, std::size_t *length_out);

    public:
        /**
         * @brief               Constructor for the Connection class that opens the socket to be used.
         * @param timeout_ms    int timeout in milliseconds after which messages will be retransmitted.
         * @throws              runtime_error if there is an error while opening the boost socket.
         */
        Connection(int timeout_ms);

        /**
         * @brief Destructor for the Connection class which stops the IO service and closes the socket.
         */
        ~Connection();

        /**
         * @brief Delete the cloning constructor so the connection can't be copied.
         */
        Connection(Connection &other) = delete;

        /**
         * @brief Delete the assignment operator so the connection can't be copied.
         */
        void operator=(const Connection &) = delete;

        /**
         * @brief       Method setEndpointLocal sets the local endpoint of the connection where packets will be received.
         * @param port  unsigned short port number that the socket should be bound to.
         * @throws      runtime_error if the socket could not be bound to the local endpoint.
         * @note        This method resets the sequence number for receiving packets.  
         */
        void setEndpointLocal(unsigned short port);

        /**
         * @brief           Method setEndpointRemote sets the remote endpoint of the connection where packets will be sent.
         * @param address   string address that the packets should be sent to.
         * @param port      unsigned short port number that the packets should be sent to.
         * @throws          runtime_error if the remote endpoint could not be set.
         * @note            This method resets the sequence number for sending packets.  
         */
        void setEndpointRemote(std::string address, unsigned short port);

        /**
         * @brief Method resetConnectionReceive resets the sequence number of all the receive channels.
         */
        void resetConnectionReceive();

        /**
         * @brief Method resetConnectionSend resets the sequence number of the send channel to 0.
         */
        void resetConnectionSend();

        /**
         * @brief       Method send sends the data contained in the buffer to the remote endpoint that was previously set.
         * @param buf   char * buffer that contains the data to be sent.
         * @param len   int length in bytes of the data contained in buf. 
         * @return      int number of bytes successfully sent to the remote endpoint.
         * @throws      runtime_error if 
         *                  - remote endpoint has not been set, 
         *                  - timeout of the socket could not be set, 
         *                  - data could not be written to the send buffer properly,
         *                  - an error occured while sending the packet using Boost ASIO,
         *                  - an error occured while receiving an ACK from the remote endpoint.
         * @note        The method will continue to re-send the message until an ACK with the correct sequence number is received. 
         */
        int send(const char *buf, int len);

        /**
         * @brief           Method receive receives a packet that another Connection object has sent to the 
         *                  previously specified local endpoint then replies with an ACK. 
         * @param buf       [out]   char * buffer to which the received data will be written.  
         * @param len       [in]    int length of the provided buffer in bytes.
         * @param address   [out]   char * address from which the packet was received.
         * @param port      [out]   int * port from which the packet was received.
         * @return          int number of bytes written to the buffer.
         * @throws          runtime_error if
         *                      - local endpoint has not been set,
         *                      - timeout of the socket could not be set, 
         *                      - if the buffer length is not long enough to accomodate the received data,
         *                      - the received data could not be copied to the buffer.
         * @note            The method will send ACKs for messages with sequence numbers less than the current sequence but will 
         *                  not return and populate the output arguments until a message with the correct sequence number is received.
         */
        int receive(char *buf, int len, char *address, int *port);
    };
}

#endif /* CONNECTION_HPP */