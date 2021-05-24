#pragma once

#include "TCPConnection.hpp"

#include "boost/asio.hpp"
#include "boost/thread.hpp"
#include "boost/bind.hpp"

using boost::asio::ip::tcp;

namespace dataManagerServer {
class DataManagerTCPServer {
public:
	DataManagerTCPServer(boost::asio::io_context& io_context, uint16_t port);

	void Stop();

	bool Status();

private:
	boost::asio::io_context& io_context_;
	tcp::acceptor acceptor_;

	void startAccept();

	void handleAccept(dataManagerServer::TCPConnection::pointer new_connection, const boost::system::error_code& error);

};
}
