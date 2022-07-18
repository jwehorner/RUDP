#include <iostream>
#include <string>
#include <thread>

#include "rudp_macros.h"
#include "Connection.hpp"
#include "ConnectionController.hpp"

using namespace std;
using namespace rudp;

int test_basic_connection();
void test_basic_connection_send_thread(bool *success);
void test_basic_connection_recv_thread(bool *success);
int test_basic_controller();
void test_basic_controller_send_thread(bool *success, ConnectionController *controller);
void test_basic_controller_recv_thread(bool *success, ConnectionController *controller);
int test_out_of_sync_connection();
void test_out_of_sync_connection_send_thread(bool *success);
void test_out_of_sync_connection_recv_thread(bool *success);
int test_multi_connection();
void test_multi_connection_send_thread(unsigned short port, bool *success);
void test_multi_connection_recv_thread(bool *success);

int main()
{
	int tests_passed = test_basic_connection();
	cout << "Test basic connection passed " << tests_passed << "/1 test cases." << endl;
	tests_passed = test_basic_controller();
	cout << "Test basic controller passed " << tests_passed << "/1 test cases." << endl;
	tests_passed = test_multi_connection();
	cout << "Test multi connection passed " << tests_passed << "/1 test cases." << endl;
	// int tests_passed = test_out_of_sync_connection();
	// cout << "Test out of sync connection passed " << tests_passed << "/1 test cases." << endl;
}

int test_basic_connection()
{
	int tests_passed = 0;
	bool send_successful = false;
	bool recv_successful = false;
	thread thread_send(test_basic_connection_send_thread, &send_successful);
	thread thread_recv(test_basic_connection_recv_thread, &recv_successful);
	thread_send.join();
	thread_recv.join();
	if (send_successful && recv_successful)
		tests_passed += 1;
	return tests_passed;
}

void test_basic_connection_send_thread(bool *success)
{
	try
	{
		Connection connection_send = Connection(1000);
		connection_send.setEndpointRemote("127.0.0.1", 3200);
		string message = "Hello World!";
		const char *send_buffer = message.c_str();
		connection_send.send(send_buffer, strlen(message.c_str()));
		connection_send.send(send_buffer, strlen(message.c_str()));
		*success = true;
	}
	catch (runtime_error error)
	{
		cout << error.what() << endl;
		*success = false;
	}
}

void test_basic_connection_recv_thread(bool *success)
{
	try
	{
		Connection connection_recv = Connection(500);
		connection_recv.setEndpointLocal(3200);
		char *recv_buffer = new char[64];
		char *address_buffer = new char[IPV4_ADDRESS_LENGTH_BYTES];
		int port;
		connection_recv.receive(recv_buffer, 64, address_buffer, &port);
		Sleep(1000);
		connection_recv.receive(recv_buffer, 64, address_buffer, &port);
		*success = true;
	}
	catch (runtime_error error)
	{
		cout << error.what() << endl;
		*success = false;
	}
}

int test_basic_controller()
{
	int tests_passed = 0;
	bool send_successful = false;
	bool recv_successful = false;
	ConnectionController *controller = ConnectionController::getInstance();
	thread thread_send(test_basic_controller_send_thread, &send_successful, controller);
	thread thread_recv(test_basic_controller_recv_thread, &recv_successful, controller);
	thread_send.join();
	thread_recv.join();
	if (send_successful && recv_successful)
		tests_passed += 1;
	return tests_passed;
}

void test_basic_controller_send_thread(bool *success, ConnectionController *controller)
{
	try
	{
		int connection_number = controller->addConnection(500);
		Connection *connection_send = controller->getConnection(connection_number);
		connection_send->setEndpointRemote("127.0.0.1", 3200);
		string message = "Hello World!";
		const char *send_buffer = message.c_str();
		connection_send->send(send_buffer, strlen(message.c_str()));
		connection_send->send(send_buffer, strlen(message.c_str()));
		*success = true;
	}
	catch (runtime_error error)
	{
		cout << error.what() << endl;
		*success = false;
	}
}

void test_basic_controller_recv_thread(bool *success, ConnectionController *controller)
{
	try
	{
		int connection_number = controller->addConnection(500);
		Connection *connection_recv = controller->getConnection(connection_number);
		connection_recv->setEndpointLocal(3200);
		char *recv_buffer = new char[64];
		char *address_buffer = new char[IPV4_ADDRESS_LENGTH_BYTES];
		int port;
		connection_recv->receive(recv_buffer, 64, address_buffer, &port);
		Sleep(1000);
		connection_recv->receive(recv_buffer, 64, address_buffer, &port);
		*success = true;
	}
	catch (runtime_error error)
	{
		cout << error.what() << endl;
		*success = false;
	}
}

int test_out_of_sync_connection()
{
	int tests_passed = 0;
	bool send_successful = false;
	bool recv_successful = false;
	thread thread_recv(test_out_of_sync_connection_recv_thread, &recv_successful);
	thread thread_send_1(test_out_of_sync_connection_send_thread, &send_successful);
	thread_send_1.join();
	Sleep(5000);
	thread thread_send_2(test_out_of_sync_connection_send_thread, &recv_successful);
	thread_send_2.join();
	thread_recv.join();
	if (send_successful && recv_successful)
		tests_passed += 1;
	return tests_passed;
}

void test_out_of_sync_connection_send_thread(bool *success)
{
	try
	{
		Connection connection_send = Connection(500);
		connection_send.setEndpointRemote("127.0.0.1", 3200);
		string message = "Hello World!";
		const char *send_buffer = message.c_str();
		connection_send.send(send_buffer, strlen(message.c_str()));
		*success = true;
	}
	catch (runtime_error error)
	{
		cout << error.what() << endl;
		*success = false;
	}
}

void test_out_of_sync_connection_recv_thread(bool *success)
{
	try
	{
		Connection connection_recv = Connection(500);
		connection_recv.setEndpointLocal(3200);
		char *recv_buffer = new char[64];
		char *address_buffer = new char[IPV4_ADDRESS_LENGTH_BYTES];
		int port;
		connection_recv.receive(recv_buffer, 64, address_buffer, &port);
		connection_recv.receive(recv_buffer, 64, address_buffer, &port);
		*success = true;
	}
	catch (runtime_error error)
	{
		cout << error.what() << endl;
		*success = false;
	}
}

int test_multi_connection()
{
	int tests_passed = 0;
	bool send_successful = false;
	bool recv_successful = false;
	thread thread_recv(test_multi_connection_recv_thread, &recv_successful);
	thread thread_send_1(test_multi_connection_send_thread, (unsigned short)3204, &send_successful);
	thread thread_send_2(test_multi_connection_send_thread, (unsigned short)3205, &recv_successful);
	thread_send_1.join();
	thread_send_2.join();
	thread_recv.join();
	if (send_successful && recv_successful)
		tests_passed += 1;
	return tests_passed;
}

void test_multi_connection_send_thread(unsigned short port, bool *success)
{
	try
	{
		Connection connection_send = Connection(500);
		connection_send.setEndpointLocal(port);
		connection_send.setEndpointRemote("127.0.0.1", 3210);
		string message = "Hello World!";
		const char *send_buffer = message.c_str();
		connection_send.send(send_buffer, strlen(message.c_str()));
		connection_send.send(send_buffer, strlen(message.c_str()));
		*success = true;
	}
	catch (runtime_error error)
	{
		cout << error.what() << endl;
		*success = false;
	}
}

void test_multi_connection_recv_thread(bool *success)
{
	try
	{
		Connection connection_recv = Connection(500);
		connection_recv.setEndpointLocal(3210);
		char *recv_buffer = new char[64];
		char *address_buffer = new char[IPV4_ADDRESS_LENGTH_BYTES];
		int port;
		connection_recv.receive(recv_buffer, 64, address_buffer, &port);
		connection_recv.receive(recv_buffer, 64, address_buffer, &port);
		connection_recv.receive(recv_buffer, 64, address_buffer, &port);
		connection_recv.receive(recv_buffer, 64, address_buffer, &port);
		*success = true;
	}
	catch (runtime_error error)
	{
		cout << error.what() << endl;
		*success = false;
	}
}
