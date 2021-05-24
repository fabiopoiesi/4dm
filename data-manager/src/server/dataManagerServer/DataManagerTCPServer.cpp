#include "TCPConnection.hpp"

#include <chrono>

#include "DataManagerTCPServer.hpp"
#include "session/SessionsManager.hpp"

#define MILLISECOND_WAIT 120000
#define WAIT_INTERVAL 50
#define MESSAGE_INTERVAL 2000

using boost::asio::ip::tcp;

dataManagerServer::DataManagerTCPServer::DataManagerTCPServer(boost::asio::io_context& io_context, uint16_t port)
		: io_context_(io_context),
		  acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
{
		this->startAccept();
		std::cout << "Start accept connection to data manager on port " << port << std::endl;
}

void dataManagerServer::DataManagerTCPServer::startAccept()
{
	TCPConnection::pointer new_connection = TCPConnection::create(io_context_);
	this->acceptor_.async_accept(new_connection->socket(),
			boost::bind(&DataManagerTCPServer::handleAccept, this, new_connection,
					boost::asio::placeholders::error));
}

bool dataManagerServer::DataManagerTCPServer::Status()
{
	return true;
	// return this->acceptor_.is_open() && this->io_context_.stopped();
}

void dataManagerServer::DataManagerTCPServer::handleAccept(dataManagerServer::TCPConnection::pointer new_connection, const boost::system::error_code& error)
{
	if (!error)
	{
		new_connection->start();
	}

	this->startAccept();
}

void dataManagerServer::DataManagerTCPServer::Stop()
{
	this->acceptor_.close();
	int waitCounter = 0;
	session::SessionsManager* instance = session::SessionsManager::getInstance();
	bool noMoreActiveSession = false;
	std::chrono::steady_clock::time_point lastMessage = std::chrono::steady_clock::now();
	while(waitCounter < MILLISECOND_WAIT && noMoreActiveSession == false) {
		if(instance->activeSession() == 0) {
			noMoreActiveSession = true;
		} else {
			std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
			std::chrono::duration<float> durationSec = currentTime - lastMessage;
			std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(durationSec);
			if(MESSAGE_INTERVAL < ms.count()) {
				std::cout << "Try to graceful stop server.." << std::endl;
				lastMessage = currentTime;
			}
			waitCounter += WAIT_INTERVAL;
			boost::this_thread::sleep(boost::posix_time::milliseconds(WAIT_INTERVAL));
		}
	}
	this->io_context_.stop();
	if(noMoreActiveSession == false) {
		std::cout << "Graceful stop impossible, server killed" << std::endl;
	}
	std::cout << "Server stopped" << std::endl;
}
