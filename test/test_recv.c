#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "rudp.h"

int main() {
	errno_t error;
	int connection_number = rudp_make_connection(DEFAULT_TIMEOUT_MS, &error);
	if (error) {
		printf("Could not open connection.\n");
		return error;
	}
	printf("Connection made with number: %d\n", connection_number);
	rudp_set_local_endpoint(connection_number, 23000, &error);
	if (error) {
		printf("Could not set local endpoint.\n");
		return error;
	}
	char buffer[64];
	char address[IPV4_ADDRESS_LENGTH_BYTES];
	int port;
	int received_size = rudp_receive(connection_number, buffer, 64, address, &port, &error);
	if (error || received_size <= 0) {
		printf("Error receiving packet: %d\n", error);
		return error;
	}
	char *message = malloc(received_size + 1);
	memcpy(message, buffer, received_size);
	message[received_size] = '\0';
	printf("Received packet on port 23000 from %s:%d\n", address, port, message);
	printf("Received packet on port 23000 from %s:%d with data: %s\n", address, port, message);
}