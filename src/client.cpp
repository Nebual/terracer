#include <stdio.h>

#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>

#include "connection.h"
#include "client.h"

static boost::asio::io_service io_service_static;

ClientSocket::ClientSocket(char ip[], char port[]) : 
			io_service_(io_service_static), 
			Connection(io_service_static, boost::asio::ip::tcp::resolver(io_service_static).resolve({ip, port})),
			thread([](){ io_service_static.run(); }) {
}

void ClientSocket::parsePacket(const Packet& msg) {
	//printf("CL: Got a packet\n");
	switch(Packets(msg.action)) {
		case Packets::DEBUG_TEXT:
			printf("CL: Received '%s'\n", msg.body());
			break;
	}
}

void ClientSocket::write(const Packet& msg) {
	printf("cl: sending message '%s' len(%d)\n", msg.body(), msg.body_length());
	io_service_.post(
			[this, msg]() {
				this->Connection::write(msg);
			});
}

void ClientSocket::close() {
	io_service_.post([this]() { socket_.close(); });
	thread.join();
}
