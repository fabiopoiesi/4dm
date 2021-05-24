#include "TCPConnection.hpp"

#include "PerformanceMeasureTCPServer.hpp"

#include "boost/asio.hpp"
#include "boost/thread.hpp"
#include "boost/bind.hpp"

using boost::asio::ip::tcp;

performanceServer::PerformanceMeasureTCPServer::PerformanceMeasureTCPServer(boost::asio::io_context& io_context, uint32_t testDuration, uint16_t port)
		: io_context_(io_context),
		  acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
{
		this->testDuration = testDuration;
		this->startAccept();
		std::cout << "Start accept connection to performance server on port " << port << std::endl;
}

void performanceServer::PerformanceMeasureTCPServer::startAccept()
{
	TCPConnection::pointer new_connection = TCPConnection::create(io_context_, this->testDuration);
	this->acceptor_.async_accept(new_connection->socket(),
			boost::bind(&PerformanceMeasureTCPServer::handleAccept, this, new_connection,
					boost::asio::placeholders::error));
}

bool performanceServer::PerformanceMeasureTCPServer::Status()
{
	return true;
	// return this->acceptor_.is_open() && this->io_context_.stopped();
}

void performanceServer::PerformanceMeasureTCPServer::handleAccept(performanceServer::TCPConnection::pointer new_connection, const boost::system::error_code& error)
{
	if (!error)
	{
		new_connection->start();
	}

	this->startAccept();
}

void performanceServer::PerformanceMeasureTCPServer::Stop()
{
	this->acceptor_.close();
	this->io_context_.stop();
}

