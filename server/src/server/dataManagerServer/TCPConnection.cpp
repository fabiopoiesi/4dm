#include "TCPConnection.hpp"

#define __DBG_INFO 1

#include <cstddef>
#include <iostream>
#include <algorithm>
#include <netinet/in.h>
#include <climits>

#define PACKET_MAX_SIZE 1200

#define DATA_MANAGER_STANDARD_MESSAGE_RESPONSE_CODE 0
#define DATA_MANAGER_TOO_LARGE_MESSAGE_RESPONSE_CODE 1

namespace dataManagerServer
{

TCPConnection::TCPConnection()
{
	this->messageDecapsulator = new MessageDecapsulator();

	this->lowProtocolDecapsulator = new dataManagerServer::encapsulation::Decapsulator(std::bind(&TCPConnection::onMessageCallback, this, std::placeholders::_1, , std::placeholders::_2), std::bind(&TCPConnection::onTooLargeMessage, this));
	this->endProtocolEncapsulator = new dataManagerServer::encapsulation::Encapsulator(std::bind(&TCPConnection::sendMessage, this, std::placeholders::_1, , std::placeholders::_2));
	this->
}

TCPConnection::~TCPConnection()
{
	delete this->messageDecapsulator;
	delete this->lowProtocolDecapsulator;
	delete this->endProtocolEncapsulator;
}

void TCPConnection::onMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buffer, muduo::Timestamp time)
{
	this->connection = conn;
	size_t totalBytes = buffer->readableBytes();
	const char* dataPointer = buffer->peek();
	this->lowProtocolDecapsulator->readBuffer(dataPointer, totalBytes);
	buffer->retrieve(totalBytes);

	/*
	 * Data
	 */
	size_t parsingPosition = 0;
	while (parsingPosition < totalBytes)
	{
		if (this->messageToReceive == 0)
		{
			uint32_t messageSizeNet =
				*((uint32_t *)&(dataPointer[parsingPosition]));
			this->messageSize = (size_t)ntohl(messageSizeNet);
			this->messageToReceive = this->messageSize;

			parsingPosition += sizeof(uint32_t);
			this->messageDiscarded =
				this->messageSize > MESSAGE_MAX_SIZE ? true : false;

			if (this->messageDiscarded)
			{
				std::cout
					<< "Next message discarded because too large, given size "
					<< this->messageSize << " max set size "
					<< MESSAGE_MAX_SIZE << std::endl;
				uint32_t messageType = htonl(
					DATA_MANAGER_TOO_LARGE_MESSAGE_RESPONSE_CODE);
				std::memcpy(responseBuffer, &messageType, sizeof(uint32_t));
				responseBufferSize += sizeof(uint32_t);
			}
			this->messageActualPosition = 0;
		}
		else
		{
			uint32_t endPoint = std::min(
				parsingPosition + this->messageToReceive,
				totalBytes);
			if (!this->messageDiscarded)
			{
				std::copy(&(dataPointer[parsingPosition]),
						  &(dataPointer[endPoint]),
						  &(this->protocolMessage[this->messageActualPosition]));
			}
			size_t bytesCopy = endPoint - parsingPosition;
			this->messageActualPosition += bytesCopy;
			this->messageToReceive -= bytesCopy;
			if (this->messageToReceive == 0 && !this->messageDiscarded)
			{ // Check if no other action must be executed after the copy
				std::cout << "Received message with size " << this->messageSize << std::endl;
			}
			parsingPosition = endPoint;
		}
	}
	
	if (responseBufferSize > 0)
	{
		this->connection->send(this->responseBuffer, responseBufferSize);
	}
}

void TCPConnection::connectionClose()
{
	std::cout << "Connection close" << std::endl;
}

void TCPConnection::onTooLargeMessage()
{
	this->connection->close();
}

void TCPConnection::onMessageCallback(const void *buffer, size_t bufferSize)
{
	MessageDecapsulator::Response response =
		this->messageDecapsulator->ElaborateMessage(
			this->buffer, bufferSize);
	switch (response.type)
	{
		case MessageDecapsulator::ResponseType::OK_NO_ANSWER:
			break;
		case MessageDecapsulator::ResponseType::OK_SEND_RESPONSE:
		{
			std::cout << "Response size: " << response.response->size()
					<< std::endl;
			uint32_t messageType = htonl(DATA_MANAGER_STANDARD_MESSAGE_RESPONSE_CODE);
			std::memcpy(responseBuffer + responseBufferSize, &messageType, sizeof(uint32_t));
			std::copy(
				response.response->c_str(),
				response.response->c_str() + response.response->size(),
				this->responseBuffer + responseBufferSize + sizeof(uint32_t));
			responseBufferSize += sizeof(uint32_t) + response.response->size();
		}
		break;
		case MessageDecapsulator::ResponseType::ERROR_CLOSE_SOCKET:
		default:
			this->connection->shutdown();
			break;
	}
}

void TCPConnection::sendMessage(const void *buffer, size_t bufferSize) {
	std::cout << "Response size: " << response.response->size() << std::endl;
	uint32_t messageType = htonl(DATA_MANAGER_STANDARD_MESSAGE_RESPONSE_CODE);
	std::memcpy(responseBuffer + responseBufferSize, &messageType, sizeof(uint32_t));
	std::copy(
		response.response->c_str(),
		response.response->c_str() + response.response->size(),
		this->responseBuffer + responseBufferSize + sizeof(uint32_t));
	responseBufferSize += sizeof(uint32_t) + response.response->size();
}

} // namespace dataManagerServer
