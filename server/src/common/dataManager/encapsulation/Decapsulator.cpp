/*
 * Decapsulator.cpp
 *
 *  Created on: May 15, 2019
 *      Author: mbortolon
 */

#include "Decapsulator.hpp"

#include <iostream>
#include "src/common/log.h"
#include <arpa/inet.h>

dataManagerServer::encapsulation::Decapsulator::~Decapsulator() {}

dataManagerServer::encapsulation::Decapsulator::Decapsulator(std::function<void(const void*, size_t)> onMessageCallback, std::function<void()> onTooLargePacket) {
	this->onMessageCallback = onMessageCallback;
	this->onTooLargePacket = onTooLargePacket;
}

void dataManagerServer::encapsulation::Decapsulator::readBuffer(const unsigned char* buffer, size_t bufferSize) {
	size_t parsingPosition = 0;
	while (parsingPosition < bufferSize) {
		if (this->messageToReceive == 0) {
			uint32_t messageSizeNet =
					*((uint32_t*) &(buffer[parsingPosition]));
			this->messageSize = (size_t) ntohl(messageSizeNet);
			this->messageToReceive = this->messageSize;

			parsingPosition += sizeof(uint32_t);
			this->messageDiscarded =
					this->messageSize > MESSAGE_MAX_SIZE ? true : false;

			if (this->messageDiscarded) {
				LOGD("Next message discarded because too large, given size %zu max set size %d", this->messageSize, MESSAGE_MAX_SIZE);
				this->onTooLargePacket();
			}
			this->messageActualPosition = 0;
		} else {
			uint32_t endPoint = std::min(
					parsingPosition + this->messageToReceive,
					bufferSize);
			if (!this->messageDiscarded) {
				std::copy(&(buffer[parsingPosition]),
						&(buffer[endPoint]),
						&(this->protocolMessage[this->messageActualPosition]));
			}
			size_t bytesCopy = endPoint - parsingPosition;
			this->messageActualPosition += bytesCopy;
			this->messageToReceive -= bytesCopy;
			if (this->messageToReceive == 0 && !this->messageDiscarded) { // Check if no other action must be executed after the copy
				LOGD("Received message with size %zu", this->messageSize);
				this->onMessageCallback(&(this->protocolMessage), this->messageSize);
			}
			parsingPosition = endPoint;
		}
	}
}
