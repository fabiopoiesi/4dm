// #include "TCPConnection.hpp"

#include <chrono>
#include <thread>

#include "DataManagerTCPServer.hpp"
#include "session/SessionsManager.hpp"

#define MILLISECOND_WAIT 120000
#define WAIT_INTERVAL 50
#define MESSAGE_INTERVAL 2000

dataManagerServer::DataManagerTCPServer::DataManagerTCPServer(muduo::net::EventLoop* loop, const muduo::net::InetAddress& listenAddr)
	: loop_(loop), server_(loop, listenAddr, "DataManager")
{
	this->server_.setConnectionCallback(
	        std::bind(&dataManagerServer::DataManagerTCPServer::onConnection, this, std::placeholders::_1)
	);
	//this->server_.setMessageCallback(
	//		std::bind(&dataManagerServer::DataManagerTCPServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
	//);
	server_.start();
	std::cout << "Start accept connection to data manager on port " << listenAddr.toPort() << std::endl;
}

bool dataManagerServer::DataManagerTCPServer::Status()
{
	return true;
	// return this->acceptor_.is_open() && this->io_context_.stopped();
}

void dataManagerServer::DataManagerTCPServer::onConnection(const muduo::net::TcpConnectionPtr& conn)
{
	if(conn->connected()) {
		dataManagerServer::TCPConnection* connectionElement = new dataManagerServer::TCPConnection();
		conn->setMessageCallback(
				std::bind(&dataManagerServer::TCPConnection::onMessage, connectionElement, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
		);
	} else {
		// TODO: manage TCP connection removal
		std::cout << "Connection from " << conn->peerAddress().toIpPort() << " is " << (conn->connected() ? "UP" : "DOWN") << std::endl;
	}
}

void dataManagerServer::DataManagerTCPServer::Stop()
{
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
			std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_INTERVAL));
		}
	}
	if(noMoreActiveSession == false) {
		std::cout << "Graceful stop impossible, server killed" << std::endl;
	}
	//this->loop_->quit();
	std::cout << "Server stopped" << std::endl;
}
