//	File:		Network_Test.cpp
//	Author:		Tanner Broaddus

#include "gtest/gtest.h"

#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <future>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <Device.h>

using std::cout;
using std::endl;



// Simple handling function that confirms that the client's message has been
// received.
void handle_accept(std::string request, int client_sock) {
	if (request == "Hello") {
		std::string confirm("Hello received!");
		int sendRes = send(client_sock, confirm.c_str(), confirm.size() + 1,
				0);
		if (sendRes == -1)
			cout << "Could not send to client!" << std::endl;
	}
	else {
		cout << "\"Hello\" was not received from client...\n";
		cout << "Message received from client: "  << request << endl;
	}
}



// Function to represent a client sending a message to the server and receiving
// a response.
// An integer value of 1000 should be returned from each client_send() call.
// (100 connections with 10 send/receives each)
int client_send() {
	
	int receive_count = 0;
	for (int i = 0; i < 100; i ++) {	
		int sock = socket(AF_INET, SOCK_STREAM, 0);
		if(sock == -1) {
			close(sock);
			return -1;
		}
		int port = 54000;
		std::string ipAddress = "127.0.0.1";
		sockaddr_in hint;
		hint.sin_family = AF_INET;
		hint.sin_port = htons(port);
		inet_pton(Af_INET, ipAddress.c_string(), &hint.sin_addr);

		int con_Result = connect(sock, (sockaddr*)&hint, sizeof(hint));
		if (con_Result == -1) {
			close(sock);
			return -1;
		}
		
		char buf[1024];
		for (int i = 0; i < 10; i++) {
			string message = "Hello";
			int sendRes = send(sock, message.c_str(), message.size() + 1, 0);
			if (sendRes == -1) { 
				cout << "Send failed" << endl;
				continue;
			}
			memset(buf, 0, 4096);
			int bytesrec = recv(sock, buf, 4096, 0);
			if (string(buf, bytesrec) == "Hello received!")
				receive_count++;
			else {
				break;
			}
		}
		close(sock);
	}
	return receive_count++;
}

//				NetworkTest
//----------------------------------------------------------------------------
/*
   Simple network test to ensure proper behavior with multiple clients
   connecting and sending messages to a server.
*/
TEST(DeviceTest, NetworkTest) {

	Scrybe::Options IO_Options;

	IO_Options.set_port(54000);
	IO_Options.set_tc(1);
	IO_Options.set_buffer_size(1024);
	IO_Options.set_accept_fail_limit(1);
	IO_Options.set_accept_loop_reset(10);
	IO_Options.set_add_fail_limit(1);
	IO_Options.set_add_loop_reset(10);
	IO_Options.set_max_events(10);
	IO_Options.set_max_listen(100);
	IO_Options.set_timeout(1000);

	ScrybeIO::Device IO_Device(&handle_accept, IO_Options);

	ASSERT_NE(IO_Device.set_listen(), -1) << "Failure in set_listen()";

	ASSERT_NE(IO_Device.start(), -1) << "Failure in start()";

	std::future<int> client1 = std::async(std::launch::async, client_send);
	std::future<int> client2 = std::async(std::launch::async, client_send);
	std::future<int> client3 = std::async(std::launch::async, client_send);

	std::this_thread::sleep_for(std::chrono::seconds(5));

	ASSERT_EQ(client1.get(), 1000);
	ASSERT_EQ(client2.get(), 1000);
	ASSERT_EQ(client3.get(), 1000);

	ASSERT_NE(IO_Device.stop(), -1) << "Failure in stop()";

}

int main(int argc, char **argv) {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}