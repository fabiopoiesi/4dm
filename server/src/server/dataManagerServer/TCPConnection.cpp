#include "TCPConnection.hpp"

#include "boost/chrono.hpp"
#include "boost/bind.hpp"

#include <string>
#include <cstddef>
#include <iostream>
#include <string>
#include <algorithm>
#include <fstream>
#include <netinet/in.h>
#include <climits>

#define PACKET_MAX_SIZE 1200

#define DATA_MANAGER_STANDARD_MESSAGE_RESPONSE_CODE 0
#define DATA_MANAGER_TOO_LARGE_MESSAGE_RESPONSE_CODE 1

using boost::asio::ip::tcp;

namespace dataManagerServer {

TCPConnection::pointer TCPConnection::create(
		boost::asio::io_context& io_context) {
	return TCPConnection::pointer(new TCPConnection(io_context));
}

tcp::socket& TCPConnection::socket() {
	return this->socket_;
}

void TCPConnection::start() {
	std::cout << "[Start] New connection request" << std::endl;
	this->continuousReading();
}

TCPConnection::TCPConnection(boost::asio::io_context& io_context) :
		socket_(io_context) {
	this->decapsulator = new MessageDecapsulator();
}

TCPConnection::~TCPConnection() {
	delete this->decapsulator;
}

void TCPConnection::continuousReading() {
	this->socket_.async_read_some(boost::asio::buffer(dataBuffer, maxLength),
			boost::bind(&TCPConnection::handleReadBytes, shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
}

void TCPConnection::handleWrite(const boost::system::error_code& error,
		size_t bytes_transferred) {
	if (error) {
		std::cout << "Error during reading, error code: " << error << ", error message: " << error.message() << std::endl;
		if(this->socket_.is_open())
		{
			this->socket_.shutdown(tcp::socket::shutdown_both);
			this->socket_.close();
		}
	}
}

void TCPConnection::handleReadBytes(const boost::system::error_code& error,
		size_t bytes_transferred) {
	if (!error) {
		// No error
		this->parseFrame(bytes_transferred);
	} else {
		if(error == boost::asio::error::eof) {
			std::cout << "Client close connection" << std::endl;
			if(this->socket_.is_open())
			{
				this->socket_.shutdown(tcp::socket::shutdown_both);
				this->socket_.close();
			}

		} else if(error == boost::asio::error::bad_descriptor) {
			std::cout << "Bad descriptor error" << std::endl;
			if(this->socket_.is_open())
			{
				this->socket_.shutdown(tcp::socket::shutdown_both);
				this->socket_.close();
			}

		} else {
			std::cout << "Error during reading, error code: " << error << ", error message: " << error.message() << std::endl;
			if(this->socket_.is_open())
			{
				this->socket_.shutdown(tcp::socket::shutdown_both);
				this->socket_.close();
			}
		}
	}
}

void TCPConnection::parseFrame(size_t bytes_transferred) {
	size_t parsingPosition = 0;
	while (parsingPosition < bytes_transferred) {
		if (this->messageToReceive == 0) {
			uint32_t messageSizeNet =
					*((uint32_t*) &(this->dataBuffer[parsingPosition]));
			this->messageSize = (size_t) ntohl(messageSizeNet);
			this->messageToReceive = this->messageSize;

			parsingPosition += sizeof(uint32_t);
			this->messageDiscarded =
					this->messageSize > MESSAGE_MAX_SIZE ? true : false;

			if (this->messageDiscarded) {
				std::cout
						<< "Next message discarded because too large, given size "
						<< this->messageSize << " max set size "
						<< MESSAGE_MAX_SIZE << std::endl;
				uint32_t messageType = htonl(DATA_MANAGER_TOO_LARGE_MESSAGE_RESPONSE_CODE);
				std::memcpy(responseBuffer, &messageType, sizeof(uint32_t));
				boost::asio::async_write(this->socket_,
						boost::asio::buffer(responseBuffer,
								sizeof(uint32_t)),
						boost::bind(&TCPConnection::handleWrite,
								shared_from_this(),
								boost::asio::placeholders::error,
								boost::asio::placeholders::bytes_transferred));
			}
			this->messageActualPosition = 0;
		} else {
			uint32_t endPoint = std::min(
					parsingPosition + this->messageToReceive,
					bytes_transferred);
			if (!this->messageDiscarded) {
				std::copy(&(this->dataBuffer[parsingPosition]),
						&(this->dataBuffer[endPoint]),
						&(this->protocolMessage[this->messageActualPosition]));
			}
			size_t bytesCopy = endPoint - parsingPosition;
			this->messageActualPosition += bytesCopy;
			this->messageToReceive -= bytesCopy;
			if (this->messageToReceive == 0 && !this->messageDiscarded) { // Check if no other action must be executed after the copy
				std::cout << "Received message with size " << this->messageSize
						<< std::endl;
				MessageDecapsulator::Response response =
						this->decapsulator->ElaborateMessage(
								&(this->protocolMessage),
								this->messageSize);
				switch (response.type) {
				case MessageDecapsulator::ResponseType::OK_NO_ANSWER:
					break;
				case MessageDecapsulator::ResponseType::OK_SEND_RESPONSE:
					{
						std::cout << "Response size: " << response.response->size()
								<< std::endl;
						uint32_t messageType = htonl(DATA_MANAGER_STANDARD_MESSAGE_RESPONSE_CODE);
						std::memcpy(responseBuffer, &messageType, sizeof(uint32_t));
						std::copy (response.response->c_str(), response.response->c_str() + response.response->size(), responseBuffer + sizeof(uint32_t));
						boost::asio::async_write(this->socket_,
								boost::asio::buffer(responseBuffer,
										sizeof(uint32_t) + response.response->size()),
								boost::bind(&TCPConnection::handleWrite,
										shared_from_this(),
										boost::asio::placeholders::error,
										boost::asio::placeholders::bytes_transferred));
					}
					break;
				case MessageDecapsulator::ResponseType::ERROR_CLOSE_SOCKET:
				default:
					if(this->socket_.is_open())
					{
						this->socket_.shutdown(tcp::socket::shutdown_both);
						this->socket_.close();
					}
					break;
				}
			}
			parsingPosition = endPoint;
		}
	}
	this->continuousReading();
}

}
