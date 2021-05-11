#include "TCPConnection.hpp"

#include <netinet/in.h>
#include <stdint.h>

#include "boost/chrono.hpp"

#ifndef htonll

#define htonll(x) ((1==htonl(1)) ? (x) : ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))

#endif

#ifndef ntohll

#define ntohll(x) ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))

#endif

using boost::asio::ip::tcp;

namespace performanceServer {

TCPConnection::pointer TCPConnection::create(boost::asio::io_context& io_context, uint32_t testDuration)
{
	return TCPConnection::pointer(new TCPConnection(io_context, testDuration));
}

tcp::socket& TCPConnection::socket()
{
	return this->socket_;
}

void TCPConnection::start()
{
	u_int32_t testDurationNetwork = htonl(this->testDuration);
	boost::asio::async_write(this->socket_, boost::asio::buffer(&testDurationNetwork, sizeof(uint32_t)),
			boost::bind(&TCPConnection::handleWriteDuration, shared_from_this(),
									boost::asio::placeholders::error,
									boost::asio::placeholders::bytes_transferred));
}

TCPConnection::TCPConnection(boost::asio::io_context& io_context, uint32_t testDuration) : socket_(io_context)
{
	this->testDuration = testDuration;
}

void TCPConnection::handleWriteDuration(const boost::system::error_code& error, size_t bytes_transferred)
{
	if(!error) {
		// All good
		std::cout << "Duration send correctly, start record duration" << std::endl;
		this->continuousReading();
	} else {
		this->socket_.close();
	}
}

void TCPConnection::continuousReading()
{
	this->socket_.async_read_some(
			boost::asio::buffer(dataBuffer, maxLength),
			boost::bind(&TCPConnection::handleReadBytes,
					shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred
			)
	);
}

void TCPConnection::handleWriteResult(const boost::system::error_code& error, size_t bytes_transferred)
{
	if(error) {
		std::cout << "Error during sending, error code: " << error << std::endl;
	}
	this->socket_.close();
}

void TCPConnection::handleReadBytes(const boost::system::error_code& error, size_t bytes_transferred)
{
	if(!error) {
		if(this->dataSend == 0) {
#ifdef BOOST_CHRONO_HAS_CLOCK_STEADY
			this->initTransmission = boost::chrono::steady_clock::now();
#else
        		this->initTransmission = boost::chrono::system_clock::now();
#endif
		}
#ifdef BOOST_CHRONO_HAS_CLOCK_STEADY
                        boost::chrono::duration<double> elapsedTime = boost::chrono::steady_clock::now() - this->initTransmission;
#else
                        boost::chrono::duration<double> elapsedTime = boost::chrono::system_clock::now() - this->initTransmission;
#endif
		if(elapsedTime.count() < this->testDuration) {
			this->dataSend += bytes_transferred;
			this->continuousReading();
		} else {
			uint64_t rate = this->dataSend / this->testDuration;
			std::cout << "Client send " << this->dataSend << " bytes in " << this->testDuration << " with an average rate of " << rate << " bytes/sec" << std::endl;
			char responseData[16];
			*((uint64_t*)&(responseData[0])) = htonll(this->dataSend);
			*((uint64_t*)&(responseData[8])) = htonll(rate);
			boost::asio::async_write(socket_, boost::asio::buffer(responseData, 16),
							boost::bind(&TCPConnection::handleWriteResult,
									shared_from_this(),
									boost::asio::placeholders::error,
									boost::asio::placeholders::bytes_transferred));
		}
	} else {
		std::cout << "Error during sending, error code: " << error << std::endl;
		this->socket_.close();
	}
}

}
