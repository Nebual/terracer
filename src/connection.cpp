#include <stdio.h>

#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>

#include "connection.h"

Connection::Connection(boost::asio::io_service& io_service, boost::asio::ip::tcp::resolver::iterator endpoint_iterator) : socket_(io_service) {
	this->socket_.async_connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string("192.168.0.160"), 5042), //endpoint_iterator,
		[this](boost::system::error_code ec) {
			if (!ec) {
				printf("Connection connected.\n");
				do_read_header();
			}
			else {
				printf("OH GOD CONNECTION (%d)\n", ec.value());
			}
		});
}

Connection::Connection(boost::asio::ip::tcp::socket newSocket) : socket_(std::move(newSocket)) {

}


void Connection::write(const Packet& msg) {
	bool write_in_progress = !write_msgs_.empty();
	write_msgs_.push_back(msg);
	if (!write_in_progress) {
		do_write();
	}
}

// All the "recv()" goes on in these two functions
void Connection::do_read_header() {
	// Read 4 bytes, so we know how large the body of the packet is
	boost::asio::async_read(socket_,
			boost::asio::buffer(curPacket.data(), Packet::header_length),
			[this](boost::system::error_code ec, std::size_t /*length*/) {
				if (!ec && curPacket.decode_header()) {
					curPacket.body()[curPacket.body_length()] = 0;
					do_read_body(); // Now read the body
				}
				else {
					printf("CL: read_header error (%d)\n", ec.value());
					if(ec.value() != 0) {
						socket_.close();
					}
					else {
						do_read_header();
					}
				}
			});
}
void Connection::do_read_body() {
	// Read until we have the entire body of the packet (length known from the header)
	boost::asio::async_read(socket_,
			boost::asio::buffer(curPacket.body(), curPacket.body_length()),
			[this](boost::system::error_code ec, std::size_t /*length*/) {
				if (!ec) {
					parsePacket(curPacket); // Actually process the packet
					do_read_header(); // Loop back to waiting for a header
				}
				else {
					printf("CL: read_body error (%d)\n", ec.value());
					socket_.close();
				}
			});
}

void Connection::do_write() {
	boost::asio::async_write(socket_,
			boost::asio::buffer(write_msgs_.front().data(), write_msgs_.front().length()),
			[this](boost::system::error_code ec, std::size_t /*length*/) {
				if (!ec) {
					write_msgs_.pop_front();
					if (!write_msgs_.empty()) {
						do_write();
					}
				}
				else {
					printf("CL: do_write error (%d)\n", ec.value());
					socket_.close();
				}
			});
}
