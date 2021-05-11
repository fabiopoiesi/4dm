/*
 * Encapsulation.unit.cpp
 *
 *  Created on: June 28, 2019
 *      Author: mbortolon
 */

#include "gtest/gtest.h"

#include "src/common/dataManager/encapsulation/Encapsulator.hpp"
#include "src/common/dataManager/encapsulation/Decapsulator.hpp"

dataManagerServer::encapsulation::Decapsulator* decapsulator;
dataManagerServer::encapsulation::Encapsulator* encapsulator;

int receiveMessage = 0;
size_t packetsSize = 0;
int tooLargeMessage = 0;
std::vector<size_t> receivePacketSize;

ssize_t senderFunction(const void* buffer, size_t bufferSize) {
	decapsulator->readBuffer(static_cast<const unsigned char*>(buffer), bufferSize);
	return bufferSize;
}

void onMessage(const void* buffer, size_t bufferSize) {
	receiveMessage++;
	packetsSize += bufferSize;
	receivePacketSize.push_back(bufferSize);
	std::cout << "Message with size " << bufferSize << std::endl;
}

void tooLargePacket() {
	tooLargeMessage++;
	std::cout << "Too large packets" << std::endl;
}

void init() {
	decapsulator = new dataManagerServer::encapsulation::Decapsulator(onMessage, tooLargePacket);
	encapsulator = new dataManagerServer::encapsulation::Encapsulator(senderFunction, 0);
}

void clean() {
	delete decapsulator;
	delete encapsulator;
	receiveMessage = 0;
	packetsSize = 0;
	tooLargeMessage = 0;
	receivePacketSize.clear();
}

TEST(Encapsulation, OneMessage) {
	init();

	size_t size_msg = 30;
	unsigned char* array_msg = new unsigned char[size_msg];
	for(unsigned int c = 0; c < size_msg; c++) {
		array_msg[c] = 'a' + rand()%26;
	}
	bool result = encapsulator->sendMessage(array_msg, size_msg);
	delete array_msg;
	ASSERT_EQ(result, true);

	EXPECT_EQ(packetsSize, size_msg);
	ASSERT_EQ(receiveMessage, 1);
	ASSERT_EQ(receivePacketSize[0], size_msg);
	ASSERT_EQ(tooLargeMessage, 0);

	clean();
}

TEST(MessageParsing, MultipleMessage) {
	init();

	unsigned int packets = 5;
	size_t* packetsSizeArray = new size_t[packets];
	size_t packetsSizeTotal = 0;
	for(unsigned int i = 0; i < packets; i++) {
		size_t size_msg = rand()%300;
		packetsSizeArray[i] = size_msg;
		packetsSizeTotal += size_msg;
		unsigned char* array_msg = new unsigned char[size_msg];
		for(unsigned int c = 0; c < size_msg; c++) {
			array_msg[c] = 'a' + rand()%26;
		}
		bool result = encapsulator->sendMessage(array_msg, size_msg);
		ASSERT_EQ(result, true);
		delete array_msg;
	}

	ASSERT_EQ(receiveMessage, packets);
	EXPECT_EQ(packetsSize, packetsSizeTotal);
	for(unsigned int i = 0; i < packets; i++) {
		ASSERT_EQ(packetsSizeArray[i], receivePacketSize[i]);
	}
	ASSERT_EQ(tooLargeMessage, 0);

	clean();
}
