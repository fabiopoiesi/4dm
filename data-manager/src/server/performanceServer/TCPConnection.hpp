#pragma once

#include "TCPConnection.hpp"

#include <iostream>

#include "boost/asio.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/bind.hpp"
#include "boost/enable_shared_from_this.hpp"
#include "boost/chrono.hpp"

using boost::asio::ip::tcp;

namespace performanceServer {

class TCPConnection : public boost::enable_shared_from_this<TCPConnection> {
public:
	typedef boost::shared_ptr<TCPConnection> pointer;

	static pointer create(boost::asio::io_context& io_context, uint32_t testDuration);

	tcp::socket& socket();

	void start();

private:
	uint64_t dataSend = 0;
	uint32_t testDuration;
	tcp::socket socket_;
	enum {
		maxLength = 128000,
	};
	char dataBuffer[maxLength];

#ifdef BOOST_CHRONO_HAS_CLOCK_STEADY
	boost::chrono::steady_clock::time_point initTransmission;
#else
	boost::chrono::system_clock::time_point initTransmission;
#endif

	TCPConnection(boost::asio::io_context& io_context, uint32_t testDuration);

	void handleWriteDuration(const boost::system::error_code& error, size_t bytes_transferred);

	void continuousReading();

	void handleWriteResult(const boost::system::error_code& error, size_t bytes_transferred);

	void handleReadBytes(const boost::system::error_code& error, size_t bytes_transferred);
};

}
