/*
 * performanceServer.cpp
 *
 *  Created on: May 12, 2019
 *      Author: matteo
 */

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

#include "boost/chrono.hpp"
#include "boost/thread/thread.hpp"
#include "boost/asio.hpp"
#include "src/server/performanceServer/PerformanceMeasureTCPServer.hpp"

#ifndef htonll

#define htonll(x) ((1==htonl(1)) ? (x) : ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))

#endif

#ifndef ntohll

#define ntohll(x) ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))

#endif

#define RECEIVE_BUFFER_SIZE 100
#define PACKET_SIZE 80000

#define PORT 5959
#define TEST_DURATION 10

#define NET_SOFTERROR -1
#define NET_HARDERROR -2


performanceServer::PerformanceMeasureTCPServer* performanceServerInstance;
bool serverRun = false;

int performanceServerThreadFunction() {
	boost::asio::io_context io_context;
	performanceServerInstance = new performanceServer::PerformanceMeasureTCPServer(io_context, TEST_DURATION, PORT);
	serverRun = true;
	io_context.run();
	return 0;
}

int currentSec = 0;

int packetsSend = 0;
double average = 0.0f;

int socketWrite(int fd, const char *buf, size_t count, int prot)
{
    register ssize_t r;
    register size_t nleft = count;

    while (nleft > 0) {

    	auto start = std::chrono::steady_clock::now();
		r = write(fd, buf, nleft);
		auto end = std::chrono::steady_clock::now();

		average = average * packetsSend + std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() * 1;
		average = average / ++packetsSend;

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

int main() {
	boost::thread* threadServerPerformance = new boost::thread(&performanceServerThreadFunction);

	// Wait for connection ready
	int maxTry = 10;
	while(maxTry >= 0 && serverRun == false) {
		boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
		maxTry--;
	}

	if(serverRun != true) {
		std::cerr << "Thread not start in time" << std::endl;
	}

	int sock = 0;
	struct sockaddr_in serv_addr;
	char* message = new char[PACKET_SIZE];
	for(int c = 0; c <= PACKET_SIZE; c++) {
		message[c] = 0x0F;
	}

	sock = socket(AF_INET, SOCK_STREAM, 0);

	//int connectionFlags = fcntl(sock, F_GETFL);
	//fcntl(sock, F_SETFL, connectionFlags | O_NONBLOCK);

	if(sock < 0) {
		delete[] message;
		std::cerr << "Error during socket creation" << std::endl;
		return 1;
	}

	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	int ipConversionResult = inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
	if(ipConversionResult < 0) {
		delete[] message;
		std::cerr << "Invalid address / Address not supported" << std::endl;
		return 1;
	}

	int connectResult = connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if(connectResult < 0) {
		delete[] message;
		std::cerr << "Connection Failed" << std::endl;
		return 1;
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
		std::cerr << "No answer from server before timeout" << std::endl;
		return 1;
	}

	delete[] message;

	read(sock, buffer, RECEIVE_BUFFER_SIZE);

	uint64_t totalByteSend = ntohll(*((uint64_t*)buffer));
	uint64_t avgRate = ntohll(*((uint64_t*)(buffer + sizeof(uint64_t))));

	close(sock);

	std::cout << "\033[34m[Result]\033[0m Total bytes send: " << totalByteSend << std::endl;
	std::cout << "\033[34m[Result]\033[0m Average rate: " << avgRate << std::endl;

	performanceServerInstance->Stop();

	threadServerPerformance->interrupt();

	std::cout << "\033[34m[Result]\033[0m Writing average wait time: " << average << std::endl;
}
