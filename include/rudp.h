/**
 * @file 	rudp.h
 * @author 	James Horner (jwehorner@gmail.com)
 * @brief 	File rudp.h contains the declaration of the C interface of the RUDP library for sending and receiving packets.
 * @date 	2022-07-18
 * @copyright Copyright (c) 2022
 */
#ifndef RUDP_H
#define RUDP_H

#include "rudp_macros.h"

#ifdef __cplusplus
extern "C"
{
#endif

	/**
	 * @brief   			Function rudp_make_connection creates a connection for later defintion and use.
	 * @param   timeout_ms 	[in]	int for the length of the time to wait for an ACK before retransmission.
	 * @param 	error		[out]	int * to hold any errors that occur, 0 if none.
	 * @return  			int connection number corresponding to the newly created connection.
	 */
	int rudp_make_connection(int timeout_ms, int *error);

	/**
	 * @brief           	Function rudp_set_remote_endpoint sets the remote endpoint of the connection where packets will be sent.
	 * @param connection	[in]	int ID of the connection.
	 * @param address   	[in]	char * address that the packets should be sent to.
	 * @param port      	[in]	unsigned short port number that the packets should be sent to.
	 * @param error			[out]	int * to hold any errors that occur, 0 if none.
	 * @note            	This method resets the sequence number for sending packets.
	 */
	void rudp_set_remote_endpoint(int connection, char *address, unsigned short port, int *error);

	/**
	 * @brief       		Function rudp_set_local_endpoint sets the local endpoint of the connection where packets will be received.
	 * @param connection	[in]	int ID of the connection.
	 * @param port  		[in]	unsigned short port number that the socket should be bound to.
	 * @param error			[out]	int * to hold any errors that occur, 0 if none.
	 * @note        		This method resets the sequence number for receiving packets.
	 */
	void rudp_set_local_endpoint(int connection, unsigned short port, int *error);

	/**
	 * @brief                       Function rudp_set_send_retries_limit sets the maximum number times the connection will attempt
	 *                              to send a packet before the send is aborted.
	 * @param connection			[in]	int ID of the connection.
	 * @param send_retries_limit    [in]	int maximum number of retries before the connection is aborted.
	 * @param error					[out]	int * to hold any errors that occur, 0 if none.
	 */
	void rudp_set_send_retries_limit(int connection, int send_retries_limit, int *error);

	/**
	 * @brief 				Function rudp_reset_connection_send resets the sequence number of the send channel to 0.
	 * @param connection	[in]	int ID of the connection.
	 * @param error			[out]	int * to hold any errors that occur, 0 if none.
	 */
	void rudp_reset_connection_send(int connection, int *error);

	/**
	 * @brief 				Function rudp_reset_connection_receive resets the sequence number of the receive channel to 0.
	 * @param connection	[in]	int ID of the connection.
	 * @param error			[out]	int * to hold any errors that occur, 0 if none.
	 */
	void rudp_reset_connection_receive(int connection, int *error);

	/**
	 * @brief       		Function rudp_send sends the data contained in the buffer to the remote endpoint that was previously set.
	 * @param connection	[in]	int ID of the connection.
	 * @param buf   		[in]	char * buffer that contains the data to be sent.
	 * @param len   		[in]	int length in bytes of the data contained in buf.
	 * @param error			[out]	int * to hold any errors that occur, 0 if none.
	 * @return      		int number of bytes successfully sent to the remote endpoint.
	 */
	int rudp_send(int connection, const char *buf, int len, int *error);

	/**
	 * @brief           	Function rudp_receive receives a packet that another connection has sent to the
	 *                  	previously specified local endpoint then replies with an ACK.
	 * @param connection	[in]	int ID of the connection.
	 * @param buf       	[out]   char * buffer to which the received data will be written.
	 * @param len       	[in]    int length of the provided buffer in bytes.
	 * @param address   	[out]   char * address from which the packet was received.
	 * @param port      	[out]   int * port from which the packet was received.
	 * @param error			[out]	int * to hold any errors that occur, 0 if none.
	 * @return          	int number of bytes written to the buffer.
	 */
	int rudp_receive(int connection, char *buf, int len, char *address_remote, int *port_remote, int *error);

#ifdef __cplusplus
}
#endif
#endif /* RUDP_H */