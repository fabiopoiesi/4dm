#include "Encapsulator.hpp"

#include <iostream>

#include "src/common/log.h"
#include <arpa/inet.h>
#include <cstring>

dataManagerServer::encapsulation::Encapsulator::Encapsulator(std::function<ssize_t(const void*, size_t)> sendingFunction, uint64_t maximumRate) {
        this->sendingFunction = sendingFunction;
        this->setMaximumRate(maximumRate);
}

inline void dataManagerServer::encapsulation::Encapsulator::wait()
{
	clock_t endwait;
	endwait = clock () + this->sleepClock;
	unsigned int waitTime = 0;
	while (clock() < endwait) { waitTime++; }
#ifdef __DBG_INFO
		std::cout << "Waited " << waitTime << " to send the package" << std::endl;
#endif
}

dataManagerServer::encapsulation::Encapsulator::~Encapsulator() {}

bool dataManagerServer::encapsulation::Encapsulator::sendMessage(unsigned char* buffer, size_t size) {
#ifdef __DBG_INFO
        std::cout << "Send encapsulated array: [";
        for(unsigned int c = 0; c < 10; c++) {
                std::cout << (static_cast<int>(buffer[c]) & 0xFF) << " ";
        }
        std::cout << "... ";
        for(unsigned int c = size - 10; c < size; c++) {
                std::cout << (static_cast<int>(buffer[c]) & 0xFF) << " ";
        }
        std::cout << "]" << std::endl;
#endif
        // Don't send the packet if size is too large
        if(size >= MESSAGE_MAX_SIZE) {
                LOGE("Client try to send a too large packet");
                // TODO: write this to file
                return false;
        }
        uint32_t size_net = htonl((uint32_t) size);
        std::memcpy(this->firstPacketBuffer, &size_net, sizeof(uint32_t));

        unsigned long int byteToSend = std::min<unsigned long int>(
                        PACKET_MAX_SIZE - sizeof(uint32_t), size);
        std::memcpy(&this->firstPacketBuffer[sizeof(uint32_t)], buffer, byteToSend);
        this->wait();
        ssize_t sendBytes = this->sendingFunction(this->firstPacketBuffer, byteToSend + sizeof(uint32_t));
        size_t pointer = byteToSend;
        if (sendBytes <= 0) {
                LOGE("Impossible write to the socket, probably it's close");
                return false;
        }

        while (pointer < size) {
                byteToSend = std::min<unsigned long int>(PACKET_MAX_SIZE,
                                size - pointer);
                ssize_t sendBytes = this->sendingFunction(&buffer[pointer], byteToSend + sizeof(uint32_t));
                wait();
                pointer += byteToSend;
                if (sendBytes <= 0) {
                        LOGD("Send bytes: %zu", sendBytes);
                        return false;
                }
        }
        return true;
}

void dataManagerServer::encapsulation::Encapsulator::setMaximumRate(uint64_t maximumRate) {
        this->maximumRate = maximumRate;
        this->sleepClock = 0;
        this->maximumRate = maximumRate;
        if(maximumRate == 0) {
                this->sleepClock = 0;
                return;
        }
        this->sleepClock = (1.0f / (maximumRate / PACKET_MAX_SIZE)) * CLOCKS_PER_SEC;
	LOGD("Sleep Clock: %u", this->sleepClock);
        std::cout << "sleepClock: " << this->sleepClock << std::endl;
}

