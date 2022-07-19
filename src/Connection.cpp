/**
 * @file 	Connection.cpp
 * @author 	James Horner (jwehorner@gmail.com)
 * @brief 	File Connection.cpp contains the definition of the Connection class of the RUDP library.
 * @details The Connection class represents a virtual connection between two RUDP connection objects
 * 			based on their sequence numbers. When sending a packet using a connection, the packet will
 * 			be retransmitted until an ACK with the correct sequence is received. If a packet is
 * 			received by a connection, an ACK will be sent in responce but the function will not
 * 			return until a message with the correct sequence is received.
 * @date 	2022-07-18
 * @copyright Copyright (c) 2022
 */
#ifndef CONNECTION_CPP
#define CONNECTION_CPP

#include "Connection.hpp"

using namespace rudp;

Connection::Connection(int timeout_ms) : timeout_ms(timeout_ms)
{
	// Initialise the members and open the socket, throwing an error on failure.
	sequence_recv_map = std::map<std::string, uint16_t>();
	sequence_send = 0;
	has_endpoint_local = false;
	has_endpoint_remote = false;

	// No deadline is required until the first socket operation is started. We
	// set the deadline to positive infinity so that the actor takes no action
	// until a specific deadline is set.
	timer.expires_at(boost::posix_time::pos_infin);

	// Start the persistent actor that checks for deadline expiry.
	check_deadline();

	try
	{
		socket.open(boost::asio::ip::udp::v4());
	}
	catch (boost::system::system_error error)
	{
		std::string error_message = std::string("[RUDP] (ERROR) [INIT] Error opening socket: ") + error.what();
		throw std::runtime_error(error_message);
	}
}

Connection::~Connection()
{
	// Stop and close the networking services.
	io_service.stop();
	socket.close();
}

void Connection::setEndpointLocal(unsigned short port)
{
	// Gain access to the mutex then try to set the local endpoint and bind the socket to it.
	// Also, reset the receive sequence as a new connection is being set up.
	std::lock_guard<std::mutex> lock(io_mutex);
	try
	{
		endpoint_local = boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::any(), port);
		socket.bind(endpoint_local);
		has_endpoint_local = true;
		resetConnectionReceive();
	}
	catch (boost::system::system_error error)
	{
		std::string error_message = "[RUDP] (ERROR) [INIT] Error setting local endpoint to port " + std::to_string(port) + ": " + error.what();
		throw std::runtime_error(error_message);
	}

#ifdef DEBUG
	std::string message = "[RUDP] (DEBUG) [INIT] Local endpoint set: " + endpoint_local.address().to_string() + ":" + std::to_string(endpoint_local.port()) + "\n";
	std::cout << message;
#endif
}

void Connection::setEndpointRemote(std::string address, unsigned short port)
{
	// Gain access to the mutex then try to set the remote endpoint.
	// Also, reset the send sequence as a new connection is being set up.
	std::lock_guard<std::mutex> lock(io_mutex);
	try
	{
		endpoint_remote = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(address), port);
		has_endpoint_remote = true;
		resetConnectionSend();
	}
	catch (boost::system::system_error error)
	{
		std::string error_message = std::string("[RUDP] (ERROR) [INIT] Error setting remote endpoint: ") + error.what();
		throw std::runtime_error(error_message);
	}

#ifdef DEBUG
	std::string message = "[RUDP] (DEBUG) [INIT] Remote endpoint set: " + endpoint_remote.address().to_string() + ":" + std::to_string(endpoint_remote.port()) + "\n";
	std::cout << message;
#endif
}

void Connection::resetConnectionReceive()
{
	sequence_recv_map.clear();
}

void Connection::resetConnectionSend()
{
	sequence_send = 0;
}

int Connection::send(const char *buf, int len)
{
	std::lock_guard<std::mutex> lock(io_mutex);
	// Throw an error if the destination is unknown.
	if (!has_endpoint_remote)
	{
		std::string error_message = "[RUDP] (ERROR) [SEND] (SEQ-SEND: " + std::to_string(sequence_send) + ") Error sending packet: No remote endpoint set.";
		throw std::runtime_error(error_message);
	}
	else
	{
		boost::system::error_code err;
		size_t sent_size;
		std::ostringstream bytestream = std::ostringstream();

		bool ack_received = false;

		// Try setting the socket timeout to the one specified in the constructor.
		try
		{
			socket.set_option(boost::asio::detail::socket_option::integer<SOL_SOCKET, SO_RCVTIMEO>{timeout_ms});
		}
		catch (boost::system::system_error error)
		{
			std::string error_message = std::string("[RUDP] (ERROR) [SEND] (SEQ-SEND: " + std::to_string(sequence_send) + ") Error setting receive timeout on socket: ") + error.what();
			throw std::runtime_error(error_message);
		}

		// Write the input data to a stringstream to the string into a byte array.
		bytestream.write(static_cast<char *>(static_cast<void *>(&sequence_send)), sizeof(sequence_send));
		bytestream.write(static_cast<char *>(static_cast<void *>(&len)), sizeof(len));
		bytestream.write(buf, len);
		if (!bytestream.good())
		{
			std::string error_message = "[RUDP] (ERROR) [SEND] (SEQ-SEND: " + std::to_string(sequence_send) + ") Error copying input data to the message buffer being sent to " + endpoint_remote.address().to_string() + ":" + std::to_string(endpoint_remote.port()) + "\n";
			throw std::runtime_error(error_message);
		}

		// While a valid ACK has not been received,
		while (!ack_received)
		{
			// Transmit the data to the remote endpoint
			sent_size = socket.send_to(boost::asio::buffer(bytestream.str()), endpoint_remote, 0, err);
			if (err.value() != 0)
			{
				std::string error_message = "[RUDP] (ERROR) [SEND] (SEQ-SEND: " + std::to_string(sequence_send) + ") Error in sending packet to " + endpoint_remote.address().to_string() + ":" + std::to_string(endpoint_remote.port()) + " with error: " + err.what() + "\n";
				throw std::runtime_error(error_message);
			}
#ifdef DEBUG
			std::string message = "[RUDP] (DEBUG) [SEND] (SEQ-SEND: " + std::to_string(sequence_send) + ") Sent " + std::to_string(sent_size) + " bytes to " + endpoint_remote.address().to_string() + ":" + std::to_string(endpoint_remote.port()) + "\n";
			std::cout << message;
#endif

			// Set a deadline for the asynchronous operation.
			// timer.expires_from_now(boost::posix_time::milliseconds(timeout_ms));

			// Set up the variables that receive the result of the asynchronous
			// operation. The error code is set to would_block to signal that the
			// operation is incomplete. Asio guarantees that its asynchronous
			// operations will never fail with would_block, so any other value in
			// ec indicates completion.
			boost::system::error_code err = boost::asio::error::would_block;
			std::size_t length = 0;
			bool cancelled = false;

			unsigned short received_sequence;
			char *buffer = new char[sizeof(received_sequence)];
			// Start the asynchronous operation itself. The handle_receive function
			// used as a callback will update the ec and length variables.
			socket.async_receive(boost::asio::buffer(buffer, sizeof(received_sequence)),
								boost::bind(&Connection::handle_receive, 
									this,
									boost::asio::placeholders::error,
									boost::asio::placeholders::bytes_transferred,
									&err,
									&length
								)
							);

			// Block until the asynchronous operation has completed.
			// do
			// 	io_service.run_one();
			// while (!cancelled);

			io_service.restart();
			io_service.run_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms));
			// std::cout << io_service.stopped() << std::endl;
			if (length <= 0)
			{
				// If the socket timed out just send the packet again and wait for an ACK.
				ack_received = false;
// #ifdef DEBUG
// 				message = "[RUDP] (DEBUG) [SEND] (SEQ-SEND: " + std::to_string(sequence_send) + ") Error thrown when receiving ACK from " + endpoint_remote.address().to_string() + ":" + std::to_string(endpoint_remote.port()) + " with error: " + err.what() + "\n";
// 				std::cout << message;
// #endif
			}
			else
			{
				// If no errors occured and the socket didn't timeout, try to compare the received sequence number to the current sequence number.
				if (memcpy_s(&received_sequence, sizeof(received_sequence), buffer, sizeof(received_sequence)))
				{
					std::string error_message = "[RUDP] (ERROR) [SEND] (SEQ-SEND: " + std::to_string(sequence_send) + ") Error copying ACK sequence number received from " + endpoint_remote.address().to_string() + ":" + std::to_string(endpoint_remote.port()) + "\n";
					std::cout << error_message;
					ack_received = false;
				}
				else
				{
					ack_received = (received_sequence == sequence_send);
				}
#ifdef DEBUG
				message = "[RUDP] (DEBUG) [SEND] (SEQ-SEND: " + std::to_string(sequence_send) + ") " + std::string(ack_received ? "Received" : "Did not receive") + " ACK with correct sequence from " + endpoint_remote.address().to_string() + ":" + std::to_string(endpoint_remote.port()) + "\n";
				std::cout << message;
#endif
			}
		}
		// Once a successful ACK has been received increment the send sequence number.
		sequence_send = (sequence_send + 1) % USHRT_MAX;
		return sent_size;
	}
}

int Connection::receive(char *buf, int len, char *address, int *port)
{
	std::lock_guard<std::mutex> lock(io_mutex);
	// Throw an error if the local endpoint is unknown.
	if (!has_endpoint_local)
	{
		std::string error_message = "[RUDP] (ERROR) [RECV] Error receiving packet: No local endpoint set.\n";
		throw std::runtime_error(error_message);
	}
	else
	{
		boost::system::error_code err;
		boost::asio::ip::udp::endpoint endpoint_sender;
		std::string sender_id;

		std::vector<char> buffer(sizeof(uint16_t) + sizeof(int) + len);
		std::vector<char> char_cache;
		int offset;

		int received_len;
		unsigned short received_sequence;

		// Try setting the socket timeout to infinite.
		try
		{
			socket.set_option(boost::asio::detail::socket_option::integer<SOL_SOCKET, SO_RCVTIMEO>{0});
		}
		catch (boost::system::system_error error)
		{
			std::string error_message = std::string("[RUDP] (ERROR) [RECV] Error setting receive timeout: ") + error.what();
			throw std::runtime_error(error_message);
		}

		bool continue_receiving = true;

		// While a message with the correct sequence number has not been received without errors.
		while (continue_receiving)
		{
			bool error_occured = false;
			offset = 0;

			// Receive a packet on the local endpoint.
			int packet_size = socket.receive_from(boost::asio::buffer(buffer, buffer.capacity()), endpoint_sender, 0, err);
			if (err.value() != 0)
			{
				std::string error_message = "[RUDP] (ERROR) [RECV] (SEQ-RECV: Error in receiving packet from " + endpoint_sender.address().to_string() + ":" + std::to_string(endpoint_sender.port()) + " with error: " + err.what() + "\n";
				std::cout << error_message;
				error_occured = true;
			}
			sender_id = endpoint_sender.address().to_string() + ":" + std::to_string(endpoint_sender.port());
			if (sequence_recv_map.find(sender_id) == sequence_recv_map.end())
			{
				sequence_recv_map[sender_id] = 0;
			}
			uint16_t sequence_recv = sequence_recv_map[sender_id];

#ifdef DEBUG
			std::string message = "[RUDP] (DEBUG) [RECV] (SEQ-RECV: " + std::to_string(sequence_recv) + ") Received " + std::to_string(packet_size) + " bytes from " + endpoint_sender.address().to_string() + ":" + std::to_string(endpoint_sender.port()) + "\n";
			std::cout << message;
#endif
			if (!error_occured)
			{
				// Parse the bytes of the received sequence number.
				char_cache = std::vector<char>(sizeof(received_sequence));
				for (int i = 0; i < sizeof(received_sequence); i++)
				{
					char_cache[i] = buffer[i + offset];
				}
				offset += sizeof(received_sequence);
				if (memcpy_s(&received_sequence, sizeof(received_sequence), char_cache.data(), sizeof(received_sequence)))
				{
					std::string error_message = "[RUDP] (ERROR) [RECV] (SEQ-RECV: " + std::to_string(sequence_recv) + ") Error copying message sequence number received from " + endpoint_sender.address().to_string() + ":" + std::to_string(endpoint_sender.port()) + "\n";
					std::cout << error_message;
					error_occured = true;
				}
			}

			if (!error_occured && received_sequence == sequence_recv)
			{
				// Parse the bytes of the received length of the data.
				char_cache = std::vector<char>(sizeof(received_len));
				for (int i = 0; i < sizeof(received_len); i++)
				{
					char_cache[i] = buffer[i + offset];
				}
				offset += sizeof(int);
				if (memcpy_s(&received_len, sizeof(received_len), char_cache.data(), sizeof(int)))
				{
					std::string error_message = "[RUDP] (ERROR) [RECV] (SEQ-RECV: " + std::to_string(sequence_recv) + ") Error copying length of message received from " + endpoint_sender.address().to_string() + ":" + std::to_string(endpoint_sender.port()) + "\n";
					std::cout << error_message;
					error_occured = true;
				}
				// If the output buffer is too small the accomodate the data in the message,
				// throw an error.
				if (!error_occured && len < received_len)
				{
					std::string error_message = "[RUDP] (ERROR) [RECV] (SEQ-RECV: " + std::to_string(sequence_recv) + ") Error buffer allocated to receive message is too small to fit message from " + endpoint_sender.address().to_string() + ":" + std::to_string(endpoint_sender.port()) + "\n";
					throw std::runtime_error(error_message);
				}
			}

			// If the message had the correct sequence number and no error occured,
			if (!error_occured && received_sequence == sequence_recv)
			{
				// Parse the bytes of the received data.
				char_cache = std::vector<char>(received_len);
				for (int i = 0; i < received_len; i++)
				{
					char_cache[i] = buffer[i + offset];
				}
				offset += received_len;
				// Copy the received data into the output buffer.
				if (memcpy_s(buf, len, char_cache.data(), received_len))
				{
					std::string error_message = "[RUDP] (ERROR) [RECV] (SEQ-RECV: " + std::to_string(sequence_recv) + ") Error copying message buffer received from " + endpoint_sender.address().to_string() + ":" + std::to_string(endpoint_sender.port()) + "\n";
					throw std::runtime_error(error_message);
				}
			}

			if (!error_occured && received_sequence == sequence_recv)
			{
				// Copy the information about where the packet came from.
				*port = (int)endpoint_sender.port();
				strcpy_s(address, IPV4_ADDRESS_LENGTH_BYTES, endpoint_sender.address().to_string().c_str());
			}

			// If no errors occured send an ACK for the message with the sequence number that was received.
			if (!error_occured && received_sequence <= sequence_recv)
			{
				char *ack_buffer = new char[sizeof(unsigned short)];
				if (memcpy_s(ack_buffer, sizeof(ack_buffer), &received_sequence, sizeof(received_sequence)))
				{
					std::string error_message = "[RUDP] (ERROR) [RECV] (SEQ-RECV: " + std::to_string(sequence_recv) + ") Error copying ACK sequence number received from " + endpoint_sender.address().to_string() + ":" + std::to_string(endpoint_sender.port()) + "\n";
					std::cout << error_message;
					error_occured = true;
				}
				size_t sent_size = socket.send_to(boost::asio::buffer(ack_buffer, sizeof(ack_buffer)), endpoint_sender, 0, err);
				if (err.value() != 0)
				{
					std::string error_message = "[RUDP] (ERROR) [RECV] (SEQ-RECV: " + std::to_string(sequence_recv) + ") Error in sending ACK to " + endpoint_sender.address().to_string() + ":" + std::to_string(endpoint_sender.port()) + " with error: " + err.what() + "\n";
					std::cout << error_message;
					error_occured = true;
				}
#ifdef DEBUG
				message = "[RUDP] (DEBUG) [RECV] (SEQ-RECV: " + std::to_string(sequence_recv) + ") Sent ACK with " + std::to_string(sent_size) + " bytes to " + endpoint_sender.address().to_string() + ":" + std::to_string(endpoint_sender.port()) + "\n";
				std::cout << message;
#endif
			}

			// If an error occured or the received sequence number does not match the current sequence number, continue receiving messages.
			continue_receiving = error_occured || (received_sequence != sequence_recv);
		}
		// Once the correct message has been received increment the receive sequence number.
		sequence_recv_map[sender_id] = (sequence_recv_map[sender_id] + 1) % USHRT_MAX;
		return received_len;
	}
}

void Connection::check_deadline()
{
	// Check whether the deadline has passed. We compare the deadline against
	// the current time since a new asynchronous operation may have moved the
	// deadline before this actor had a chance to run.
	if (timer.expires_at() <= boost::asio::deadline_timer::traits_type::now())
	{
		// The deadline has passed. The outstanding asynchronous operation needs
		// to be cancelled so that the blocked receive() function will return.
		//
		// Please note that cancel() has portability issues on some versions of
		// Microsoft Windows, and it may be necessary to use close() instead.
		// Consult the documentation for cancel() for further information.
		// boost::system::error_code err = boost::asio::error::operation_aborted;
		socket.cancel();

		// There is no longer an active deadline. The expiry is set to positive
		// infinity so that the actor takes no action until a new deadline is set.
		timer.expires_at(boost::posix_time::pos_infin);
	}

	// Put the actor back to sleep.
	timer.async_wait(boost::bind(&Connection::check_deadline, this));
}

void Connection::handle_receive(const boost::system::error_code &err, std::size_t length, boost::system::error_code *err_out, std::size_t *length_out)
{
	*err_out = err;
	*length_out = length;
	io_service.stop();
}

#endif /* CONNECTION_CPP */