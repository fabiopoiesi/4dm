#pragma once

#include "TCPConnection.hpp"

#include "boost/asio.hpp"
#include "boost/thread.hpp"
#include "boost/bind.hpp"

using boost::asio::ip::tcp;

namespace performanceServer {
class PerformanceMeasureTCPServer {
public:
	PerformanceMeasureTCPServer(boost::asio::io_context& io_context, uint32_t testDuration, uint16_t port);

	void Stop();

	bool Status();

private:
	uint32_t testDuration = 30;
	boost::asio::io_context& io_context_;
	tcp::acceptor acceptor_;

	void startAccept();

	void handleAccept(performanceServer::TCPConnection::pointer new_connection, const boost::system::error_code& error);

};
}
