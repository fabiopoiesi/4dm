#ifndef SRC_DATAMANAGERSERVER_TCPCONNECTION_HPP_
#define SRC_DATAMANAGERSERVER_TCPCONNECTION_HPP_

#include "muduo/net/TcpServer.h"
#include "MessageDecapsulator.hpp"
#include "src/common/dataManager/encapsulation/Decapsulator.hpp"
#include "src/common/dataManager/encapsulation/Encapsulator.hpp"

#define MESSAGE_MAX_SIZE 300000
#define DATA_MANAGER_RESPONSE_MESSAGES_SIZE 2000

namespace dataManagerServer {

class TCPConnection {
public:
	TCPConnection();
	virtual ~TCPConnection();

	void onMessage(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buf, muduo::Timestamp time);

	void connectionClose();

private:
	char protocolMessage[MESSAGE_MAX_SIZE];
	char responseBuffer[DATA_MANAGER_RESPONSE_MESSAGES_SIZE];
	size_t messageToReceive = 0;
	unsigned int messageActualPosition = 0;
	bool messageDiscarded = false;
	size_t messageSize = 0;

	MessageDecapsulator* messageDecapsulator;

	dataManagerServer::encapsulation::Decapsulator* lowProtocolDecapsulator;
	dataManagerServer::encapsulation::Encapsulator* endProtocolEncapsulator;

	muduo::net::TcpConnectionPtr connection;

	void onTooLargeMessage();

	void onMessageCallback(const void* buffer, size_t bufferSize);
};

}

#endif
