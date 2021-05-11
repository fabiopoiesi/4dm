#pragma once

// #include "TCPConnection.hpp"

// #include "boost/asio.hpp"
// #include "boost/thread.hpp"
// #include "boost/bind.hpp"

#include "muduo/net/TcpServer.h"

// #include "muduo/base/AsyncLogging.h"
// #include "muduo/base/Logging.h"
#include "muduo/base/Thread.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/InetAddress.h"

#include <functional>
#include <utility>
#include <list>

#include <stdio.h>
#include <unistd.h>

#include "TCPConnection.hpp"


namespace dataManagerServer {
class DataManagerTCPServer {
public:
	DataManagerTCPServer(muduo::net::EventLoop* loop, const muduo::net::InetAddress& listenAddr);

	void Stop();

	bool Status();

private:
	muduo::net::EventLoop* loop_;
	muduo::net::TcpServer server_;

	void onConnection(const muduo::net::TcpConnectionPtr& conn);

	void onMessage(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buf, muduo::Timestamp time);

	// std::map<onMessage> // TODO: replace with a thread safe map
};
}
