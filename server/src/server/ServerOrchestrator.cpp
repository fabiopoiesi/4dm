#include "ServerOrchestrator.hpp"

#include <exception>
#include <iostream>
#include <csignal>

#include "boost/asio.hpp"
#include "boost/thread.hpp"

#include "muduo/net/EventLoop.h"
#include "muduo/net/InetAddress.h"

#define STARTUP_TRY 2000
#define WAIT_INTERVAL 10

using boost::asio::ip::tcp;

bool ServerOrchestrator::serverShutdownRequest = true;

void ServerOrchestrator::StopSignalHandler(int signum) {
	ServerOrchestrator::serverShutdownRequest = true;
}

void ServerOrchestrator::WaitStopSignal() {
	while(ServerOrchestrator::serverShutdownRequest == false) {

		bool status = this->performanceServerInstance->Status();
		if(status == false) {
			std::cout << "Performance server exit abnormally, closing programs..." << std::endl;
			ServerOrchestrator::serverShutdownRequest = true;
		}
	}
}

int ServerOrchestrator::performanceServerThreadFunction() {
	boost::asio::io_context io_context;
	this->performanceServerInstance = new performanceServer::PerformanceMeasureTCPServer(io_context, 12, 5959);
	this->threadServerPerformanceStarted = true;
	io_context.run();
	return 0;
}

int ServerOrchestrator::dataManagerTCPServerThreadFunction() {
	muduo::net::EventLoop loop;
	muduo::net::InetAddress listenAddr(5960);
	this->dataManagerServerInstance = new dataManagerServer::DataManagerTCPServer(&loop, listenAddr);
	this->threadDataManagerStarted = true;
	loop.loop();
	return 0;
}

bool ServerOrchestrator::Start() {
	serverShutdownRequest = false;
	signal(SIGINT, &ServerOrchestrator::StopSignalHandler);
	try {
		this->threadServerPerformance = new boost::thread(boost::bind(&ServerOrchestrator::performanceServerThreadFunction, this));
		this->threadDataManagerServer = new boost::thread(boost::bind(&ServerOrchestrator::dataManagerTCPServerThreadFunction, this));
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
		return false;
	}
	int tryRemain = STARTUP_TRY;
	while(tryRemain > 0) {
		if(this->threadDataManagerStarted && this->threadServerPerformanceStarted) {
			break;
		}
		boost::this_thread::sleep(boost::posix_time::milliseconds(WAIT_INTERVAL));
	}
	if(this->threadDataManagerStarted && this->threadServerPerformanceStarted) {
		return true;
	}
	return false;
}

void ServerOrchestrator::Stop() {
	this->performanceServerInstance->Stop();
	this->dataManagerServerInstance->Stop();
	this->threadServerPerformance->interrupt();
	this->threadDataManagerServer->interrupt();
	delete this->threadServerPerformance;
	delete this->threadDataManagerServer;
	delete this->performanceServerInstance;
	delete this->dataManagerServerInstance;
	this->threadServerPerformanceStarted = false;
	this->threadServerPerformanceStarted = false;
}
