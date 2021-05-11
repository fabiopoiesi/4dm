/*
 * MessageParsing.unit.cpp
 *
 *  Created on: May 6, 2019
 *      Author: mbortolon
 */

#include <cstddef>
#include <iostream>
#include <string>
#include <algorithm>
#include <fstream>
#include <netinet/in.h>

#include <climits>

#include "gtest/gtest.h"

#include "boost/system/error_code.hpp"

#define MESSAGE_MAX_SIZE 200000
#define TCP_BUFFER_SIZE 32768

#define PACKET_MAX_SIZE 1200

char protocolMessage[MESSAGE_MAX_SIZE];
size_t messageToReceive = 0;
unsigned int messageActualPosition = 0;
bool messageDiscarded = false;
size_t messageSize = 0;

enum {
	maxLength = TCP_BUFFER_SIZE,
};
char dataBuffer[maxLength];

char** messages;
unsigned int messageId = 0;

void parseFrame(bool error,
		size_t bytes_transferred) {
	if (!error) {
		// No error
		size_t parsingPosition = 0;
		while (parsingPosition < bytes_transferred) {
			if (messageToReceive == 0) {
				uint32_t messageSizeNet = *((uint32_t*)&(dataBuffer[parsingPosition]));
				messageSize = (size_t)ntohl(messageSizeNet);
				messageToReceive = messageSize;

				parsingPosition += sizeof(uint32_t);
				messageDiscarded =
						messageSize > MESSAGE_MAX_SIZE ? true : false;
				if(messageDiscarded) {
					std::cout << "Next message discarded because too large, given size " << messageSize << " max set size " << MESSAGE_MAX_SIZE << std::endl;
				}
				messageActualPosition = 0;
			} else {
				uint32_t endPoint = std::min(parsingPosition + messageToReceive,
						bytes_transferred);
				if(!messageDiscarded) {
					std::copy(&(dataBuffer[parsingPosition]),
							&(dataBuffer[endPoint]),
							&(protocolMessage[messageActualPosition]));
				}
				size_t bytesCopy = endPoint - parsingPosition;
				messageActualPosition += bytesCopy;
				messageToReceive -= bytesCopy;
				if (messageToReceive == 0 && !messageDiscarded) {// Check if no other action must be executed after the copy
					std::cout << "Receive message with size " << messageSize << std::endl;
					char* message = new char[messageSize];
					std::copy(&(protocolMessage[0]), &(protocolMessage[messageSize]), message);
					messages[messageId] = message;
					messageId++;
				}
				parsingPosition = endPoint;
			}
		}
	} else {
		std::cout << "Error during reading, error code: " << error << std::endl;
	}
}

void initMessages(unsigned int numberOfMessages) {
	messages = new char*[numberOfMessages];
	messageId = 0;
}

void cleanMessages() {
	for(unsigned int c = 0; c < messageId; c++) {
		delete[] messages[c];
	}
	delete[] messages;
}

TEST(MessageParsing, OneMessageOnePacket) {
	initMessages(1);

	uint32_t size_pkt1 = 30;
	uint32_t size_pkt1_net = htonl(size_pkt1);
	std::memcpy(dataBuffer, &size_pkt1_net, sizeof(uint32_t));
	char* array_pkt1 = new char[size_pkt1];
	for(unsigned int c = 0; c < size_pkt1; c++) {
		array_pkt1[c] = 'a' + rand()%26;
	}
	std::memcpy(&dataBuffer[sizeof(uint32_t)], array_pkt1, size_pkt1);

	parseFrame(false, sizeof(uint32_t) + size_pkt1);

	ASSERT_EQ(messageId, 1);	
	bool allEqual = true;
	for(unsigned int c = 0; c < size_pkt1; c++) {
		allEqual = allEqual && (array_pkt1[c] == messages[0][c]);
	}
	EXPECT_TRUE(allEqual);

	delete[] array_pkt1;
	cleanMessages();
}

TEST(MessageParsing, OneMessageMultiplePacket) {
	initMessages(1);

	size_t total_size = 1500;
	uint32_t total_size_net = htonl((uint32_t)total_size);
	char* data = new char[total_size];
	for(unsigned int c = 0; c < total_size; c++) {
		data[c] = 'a' + rand()%26;
	}

	uint32_t size_pkt1 = total_size / 2;
	//std::copy(dataBuffer, &total_size, sizeof(uint32_t));
	//std::copy(&dataBuffer[sizeof(uint32_t)], data, size_pkt1);
	std::memcpy(dataBuffer, &total_size_net, sizeof(uint32_t));
	std::memcpy(&dataBuffer[sizeof(uint32_t)], data, size_pkt1);

	parseFrame(false, sizeof(uint32_t) + size_pkt1);

	uint32_t size_pkt2 = total_size - size_pkt1;
	//std::copy(dataBuffer, &data[size_pkt1], size_pkt2);
	std::memcpy(dataBuffer, &data[size_pkt1], size_pkt2);

	parseFrame(false, size_pkt2);

	ASSERT_EQ(messageId, 1);
	bool allEqual = true;
	for(unsigned int c = 0; c < total_size; c++) {
		allEqual = allEqual && (data[c] == messages[0][c]);
	}
	EXPECT_TRUE(allEqual);

	delete[] data;
	cleanMessages();
}

TEST(MessageParsing, MultipleMessageOnePacket) {
	initMessages(2);

	uint32_t size_pkt1 = 30;
	uint32_t size_pkt1_net = htonl(size_pkt1);
	//std::copy(dataBuffer, &size_pkt1, sizeof(uint32_t));
	std::memcpy(dataBuffer, &size_pkt1_net, sizeof(uint32_t));
	char* array_pkt1 = new char[size_pkt1];
	for(unsigned int c = 0; c < size_pkt1; c++) {
		array_pkt1[c] = 'a' + rand()%26;
	}
	//std::copy(&dataBuffer[sizeof(uint32_t)], array_pkt1, size_pkt1);
	std::memcpy(&dataBuffer[sizeof(uint32_t)], array_pkt1, size_pkt1);

	uint32_t size_pkt2 = 30;
	uint32_t size_pkt2_net = htonl(size_pkt2);
	//std::copy(&dataBuffer[sizeof(uint32_t) + size_pkt1], &size_pkt2, sizeof(uint32_t));
	std::memcpy(&dataBuffer[sizeof(uint32_t) + size_pkt1], &size_pkt2_net, sizeof(uint32_t));
	char* array_pkt2 = new char[size_pkt2];
	for(unsigned int c = 0; c < size_pkt2; c++) {
		array_pkt2[c] = 'a' + rand()%26;
	}
	//std::copy(&dataBuffer[sizeof(uint32_t) + size_pkt1 + sizeof(uint32_t)], array_pkt2, size_pkt2);
	std::memcpy(&dataBuffer[sizeof(uint32_t) + size_pkt1 + sizeof(uint32_t)], array_pkt2, size_pkt2);

	parseFrame(false, sizeof(uint32_t) + size_pkt1 + sizeof(uint32_t) + size_pkt2);

	ASSERT_EQ(messageId, 2);

	bool allEqual = true;
	for(unsigned int c = 0; c < size_pkt1; c++) {
		allEqual = allEqual && (array_pkt1[c] == messages[0][c]);
	}
	for(unsigned int c = 0; c < size_pkt2; c++) {
		allEqual = allEqual && (array_pkt2[c] == messages[1][c]);
	}
	EXPECT_TRUE(allEqual);

	delete[] array_pkt1;
	delete[] array_pkt2;
	cleanMessages();
}

TEST(MessageParsing, TooBigMessage) {
	initMessages(1);

	size_t total_size = MESSAGE_MAX_SIZE + 2;
	uint32_t total_size_net = htonl((uint32_t)total_size);
	char* data = new char[total_size];
	for(unsigned int c = 0; c < total_size; c++) {
		data[c] = 'a';
	}

	std::memcpy(dataBuffer, &total_size_net, sizeof(uint32_t));

	unsigned int netBufferPosition = sizeof(uint32_t);
	unsigned int finalArrayPosition = 0;

	while(finalArrayPosition < total_size) {
		size_t bytesToSend = std::min<size_t>(PACKET_MAX_SIZE, total_size - finalArrayPosition);
		std::memcpy(&dataBuffer[netBufferPosition], &data[finalArrayPosition], bytesToSend);
		parseFrame(false, bytesToSend + netBufferPosition);
		finalArrayPosition += bytesToSend;
		netBufferPosition = 0;
	}

	EXPECT_EQ(messageId, 0);

	delete[] data;
	cleanMessages();
}

TEST(MessageParsing, BigMessageParsing) {
	initMessages(1);

	size_t total_size = 65000;
	uint32_t total_size_net = htonl((uint32_t)total_size);

	std::cout << "total_size:" << (uint32_t)total_size << std::endl;
	std::cout << "total_size_net:" << total_size_net << std::endl;

	char* data = new char[total_size];
	for(unsigned int c = 0; c < total_size; c++) {
		data[c] = 'a';
	}

	std::memcpy(dataBuffer, &total_size_net, sizeof(uint32_t));

	unsigned int netBufferPosition = sizeof(uint32_t);
	unsigned int finalArrayPosition = 0;

	while(finalArrayPosition < total_size) {
		size_t bytesToSend = std::min<size_t>(PACKET_MAX_SIZE, total_size - finalArrayPosition);
		std::memcpy(&dataBuffer[netBufferPosition], &data[finalArrayPosition], bytesToSend);
		parseFrame(false, bytesToSend + netBufferPosition);
		finalArrayPosition += bytesToSend;
		netBufferPosition = 0;
	}

	ASSERT_EQ(messageId, 1);

	bool allEqual = true;
	for(unsigned int c = 0; c < total_size; c++) {
		allEqual = allEqual && (data[c] == messages[0][c]);
	}
	EXPECT_TRUE(allEqual);

	delete[] data;
	cleanMessages();
}
