#ifndef SRC_DATAMANAGERSERVER_TCPCONNECTION_HPP_
#define SRC_DATAMANAGERSERVER_TCPCONNECTION_HPP_

#include <iostream>
#include <string>

#include "boost/asio.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/enable_shared_from_this.hpp"
#include "MessageDecapsulator.hpp"

#define MESSAGE_MAX_SIZE 400000
#define TCP_BUFFER_SIZE 128000
#define DATA_MANAGER_RESPONSE_MESSAGES_SIZE 2000

using boost::asio::ip::tcp;

namespace dataManagerServer {

class TCPConnection : public boost::enable_shared_from_this<TCPConnection> {
public:
	typedef boost::shared_ptr<TCPConnection> pointer;

	static pointer create(boost::asio::io_context& io_context);

	tcp::socket& socket();

	void start();

	virtual ~TCPConnection();

private:
	tcp::socket socket_;
	enum {
		maxLength = TCP_BUFFER_SIZE,
	};
	char dataBuffer[maxLength];

	char protocolMessage[MESSAGE_MAX_SIZE];
	char responseBuffer[DATA_MANAGER_RESPONSE_MESSAGES_SIZE];
	size_t messageToReceive = 0;
	unsigned int messageActualPosition = 0;
	bool messageDiscarded = false;
	size_t messageSize = 0;

	TCPConnection(boost::asio::io_context& io_context);

	void continuousReading();

	void handleReadBytes(const boost::system::error_code& error, size_t bytes_transferred);

	void handleWrite(const boost::system::error_code& error, size_t bytes_transferred);

	void parseFrame(size_t bytes_transferred);

	MessageDecapsulator* decapsulator;
};

}

#endif
