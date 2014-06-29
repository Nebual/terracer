#pragma once

#include "connection.h"
#include "packet.h"

class ClientSocket : Connection {
public:
	ClientSocket(char ip[], char port[]);
	void close();
	void write(const Packet& msg);
private:
	void parsePacket(const Packet& msg);
private:
	boost::asio::io_service& io_service_;
	boost::thread thread;
};
