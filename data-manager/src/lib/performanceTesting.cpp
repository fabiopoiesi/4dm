/*
 * performanceTesting.cpp
 *
 *  Created on: May 15, 2019
 *      Author: mbortolon
 */

#include "performanceTesting.h"

#include <iostream>

#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

#include <stdint.h>
#include <string>
#include <chrono>
#include <unistd.h>

#include "log.h"

#ifndef htonll

#define htonll(x) ((1==htonl(1)) ? (x) : ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))

#endif

#ifndef ntohll

#define ntohll(x) ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))

#endif

#define RECEIVE_BUFFER_SIZE 100
#define PACKET_SIZE 128000

#define NET_SOFTERROR -1
#define NET_HARDERROR -2

namespace Performance {
	bool status;
	const char* errorMessage;
	uint64_t avgRate;
	uint64_t totalByteSend;

	uint64_t packetsSend = 0;
	double averageWriteTime = 0.0f;

	extern "C" bool getStatus() {
		return status;
	}

	extern "C" char* getErrorMessage() {
		if(errorMessage == nullptr)
			return nullptr;
		char* errorCopy = new char[strlen(errorMessage)];
		strcpy(errorCopy, errorMessage);
		return errorCopy;
	}

	extern "C" uint64_t getAvgRate() {
		return avgRate;
	}

	extern "C" uint64_t getTotalByteSend() {
		return totalByteSend;
	}

	extern "C" uint32_t getAverageWriteTimeInNs() {
		return static_cast<uint32_t>(averageWriteTime);
	}

	extern "C" uint64_t getPacketsSend() {
		return packetsSend;
	}

	int socketWrite(int fd, const char *buf, size_t count, int prot)
	{
		ssize_t r;
		size_t nleft = count;

		while (nleft > 0) {
			auto start = std::chrono::steady_clock::now();
			r = write(fd, buf, nleft);
			auto end = std::chrono::steady_clock::now();

			averageWriteTime = averageWriteTime * packetsSend + std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() * 1;
			averageWriteTime = averageWriteTime / ++packetsSend;
			if (r < 0) {
				switch (errno) {
				case EINTR:
				case EAGAIN:
		#if (EAGAIN != EWOULDBLOCK)
				case EWOULDBLOCK:
		#endif
				return count - nleft;

				case ENOBUFS:
				return NET_SOFTERROR;

				default:
				return NET_HARDERROR;
				}
			} else if (r == 0) {
				return NET_SOFTERROR;
			}
			nleft -= r;
			buf += r;
		}
		return count;
	}

	extern "C" void performanceTesting(const char* serverIpAddress, uint32_t serverPort) {
		status = false;
		avgRate = 0;
		totalByteSend = 0;

		int sock = 0;
		struct sockaddr_in serv_addr;
		char* message = new char[PACKET_SIZE];
		for(int c = 0; c <= PACKET_SIZE; c++) {
			message[c] = 0x0F;
		}

		sock = socket(AF_INET, SOCK_STREAM, 0);

		if(sock < 0) {
			errorMessage = "Error during socket creation";
			delete[] message;
			return;
		}

		memset(&serv_addr, '0', sizeof(serv_addr));

		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(serverPort);

		int ipConversionResult = inet_pton(AF_INET, serverIpAddress, &serv_addr.sin_addr);
		if(ipConversionResult < 0) {
			errorMessage = "Invalid address / Address not supported";
			delete[] message;
			return;
		}

		int connectResult = connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
		if(connectResult < 0) {
			errorMessage = "Connection Failed";
			delete[] message;
			return;
		}

		char buffer[RECEIVE_BUFFER_SIZE] = {0};
		read(sock, buffer, RECEIVE_BUFFER_SIZE);

		uint32_t duration = ntohl(*((uint32_t*)buffer));

		std::chrono::steady_clock::time_point initTime = std::chrono::steady_clock::now();
		std::chrono::duration<double> remain = std::chrono::steady_clock::now() - initTime;

		while(remain.count() < duration) {
			int result = socketWrite(sock, message, PACKET_SIZE, SOCK_STREAM);

			remain = std::chrono::steady_clock::now() - initTime;
		}

		int receiveElement = 0;
		int remainTest = 1000;
		while(receiveElement == 0 && remainTest > 0) {
			ioctl(sock, FIONREAD, &receiveElement);
			send(sock, message, PACKET_SIZE, 0);
			remainTest--;
		}

		if(remainTest == 0) {
			errorMessage = "No answer from server before timeout";
			delete[] message;
			return;
		}

		delete[] message;

		read(sock, buffer, RECEIVE_BUFFER_SIZE);

		status = true;
		totalByteSend = ntohll(*((uint64_t*)buffer));
		avgRate = ntohll(*((uint64_t*)(buffer + sizeof(uint64_t))));
		errorMessage = nullptr;

		close(sock);

		return;
	}
}
