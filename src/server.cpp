#include <stdio.h>

#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/lexical_cast.hpp>

#include "connection.h"
#include "server.h"

ServerSocket::ServerSocket(int port) : 
			svAcceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
			svSocket(io_service) {
	printf("SV: Starting listening on %d\n", port);

	boost::thread([this](){ this->io_service.run(); }).swap(this->thread);
	do_accept();
}
void ServerSocket::do_accept() {
	svAcceptor.async_accept(svSocket,
			[this](boost::system::error_code ec)  {
				if (!ec) {
					printf("SV: Client connected (%s)!\n", boost::lexical_cast<std::string>(svSocket.remote_endpoint()).c_str());
					this->clients.emplace_back(std::move(svSocket));
					this->clients.back().start();
				} else {
					printf("SV: Accept failed (%d)\n", ec.value());
				}
				do_accept();
			});
}

void ServerSocket::close() {
	io_service.post([this]() { 
		for(auto &client : clients) {
			client.close();
		}
		svAcceptor.close();
		svSocket.close(); 
		io_service.stop();
	});
	thread.join();
}

ClientConnection::ClientConnection(boost::asio::ip::tcp::socket newSocket) : Connection(std::move(newSocket)) {

}
void ClientConnection::start() {
	do_read_header();
}
void ClientConnection::close() {
	socket_.close();
}
void ClientConnection::parsePacket(const Packet& msg) {
	printf("SV: Got a packet '%s', %d, sending back\n", msg.body(), msg.body_length());
	write(msg);
}
