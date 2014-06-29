#pragma once

#include <vector>
#include "connection.h"
#include "packet.h"

class ClientConnection;

class ServerSocket {
public:
	std::vector<ClientConnection> clients;
	
	ServerSocket(int port = 5042);
	void close();
private:
	void do_accept();
private:
	boost::asio::io_service io_service;
	boost::thread thread;
	boost::asio::ip::tcp::acceptor svAcceptor;
	boost::asio::ip::tcp::socket svSocket;
};

class ClientConnection : Connection {
public:
	ClientConnection(boost::asio::ip::tcp::socket newSocket);
	void start();
	void close();
private:
	void parsePacket(const Packet& msg);
private:
};
