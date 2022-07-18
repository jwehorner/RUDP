#include <stdio.h>

#include "rudp.h"

int main() {
	errno_t error;
	int connection_number = rudp_make_connection(DEFAULT_TIMEOUT_MS, &error);
	if (error) {
		printf("Could not open connection.\n");
		return error;
	}
	printf("Connection made with number: %d\n", connection_number);
	rudp_set_remote_endpoint(connection_number, "127.0.0.1", 23000, &error);
	if (error) {
		printf("Could not set local endpoint.\n");
		return error;
	}
	char data[] = "Hello World!";
	rudp_send(connection_number, data, sizeof(data), &error);
	if (error) {
		printf("Error sending packet: %d\n", error);
		return error;
	}
	printf("Sent packet to 23000 with %d bytes of data: %s\n", (int)sizeof(data), data);
}