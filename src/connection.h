#pragma once

#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <deque>
#include "packet.h"

class Connection {
public:
	virtual void write(const Packet& msg);
protected:
	Connection(boost::asio::io_service& io_service, boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
	Connection(boost::asio::ip::tcp::socket newSocket);
	void do_read_header();
	void do_read_body();
	void do_write();
	virtual void parsePacket(const Packet& msg) {printf("Got a packssssset\n");};
protected:
	Packet curPacket;
	boost::asio::ip::tcp::socket socket_;
	std::deque<Packet> write_msgs_;
};
