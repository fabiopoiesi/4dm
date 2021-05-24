#include "gtest/gtest.h"
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

#include "boost/chrono.hpp"
#include "boost/thread/thread.hpp"
#include "src/server/performanceServer/PerformanceMeasureTCPServer.hpp"

#define PORT 5959
#define TEST_DURATION 2

#ifndef htonll

#define htonll(x) ((1==htonl(1)) ? (x) : ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))

#endif

#ifndef ntohll

#define ntohll(x) ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))

#endif

performanceServer::PerformanceMeasureTCPServer* performanceServerInstance;
bool serverRun = false;

int performanceServerThreadFunction() {
	boost::asio::io_context io_context;
	performanceServerInstance = new performanceServer::PerformanceMeasureTCPServer(io_context, TEST_DURATION, PORT);
	serverRun = true;
        io_context.run();
	return 0;
}

TEST(PerformanceServer, TestCalculationCorrectness) {
    boost::thread* threadServerPerformance = new boost::thread(&performanceServerThreadFunction);

    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char *message = new char[1500];
    char buffer[150] = {0};
    for(int c = 0; c <= 1500; c++) {
        message[c] = 0xFF;
    }

    ASSERT_GE(sock = socket(AF_INET, SOCK_STREAM, 0), 0) << "Socket creation error";

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    ASSERT_GT(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr), 0) << "Invalid address/ Address not supported";

    // Wait for connection ready
    int maxTry = 10;
    while(maxTry >= 0 && serverRun == false) {
        boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
        maxTry--;
    }
    
    ASSERT_TRUE(serverRun) << "Thread not start in time";

    ASSERT_GE(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)), 0) << "Connection Failed";

    int count;

    int numberTry = 0;
    while(numberTry < 20 && count == 0) {
        ioctl(sock, FIONREAD, &count);
        numberTry++;
        boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
    }

    ASSERT_NE(count, 0) << "No data available on response socket";

    valread = read(sock, buffer, 150);

    ASSERT_EQ(valread, 4) << "Expected 4 bytes, but " << valread << " bytes returned";

    EXPECT_EQ(ntohl(*((uint32_t*)buffer)), TEST_DURATION);

#ifdef BOOST_CHRONO_HAS_CLOCK_STEADY
    boost::chrono::steady_clock::time_point initTime = boost::chrono::steady_clock::now();
#else
    boost::chrono::system_clock::time_point initTime = boost::chrono::system_clock::now();
#endif
    bool sendingResult = true;
    ssize_t result = send(sock, message, 1500, 0);
    sendingResult = result && result >= 0;
    result = send(sock, message, 1345, 0);
    sendingResult = result && result >= 0;
    result = send(sock, message, 1200, 0);
    sendingResult = result && result >= 0;

    EXPECT_EQ(sendingResult, true);

#ifdef BOOST_CHRONO_HAS_CLOCK_STEADY
    boost::chrono::duration<double> remain = boost::chrono::steady_clock::now() - initTime;
#else
    boost::chrono::duration<double> remain = boost::chrono::system_clock::now() - initTime;
#endif

    while(remain.count() < TEST_DURATION + 0.1d) {
        boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
#ifdef BOOST_CHRONO_HAS_CLOCK_STEADY
        remain = boost::chrono::steady_clock::now() - initTime;
#else
        remain = boost::chrono::system_clock::now() - initTime;
#endif
    }
    std::cout << std::endl;
    ssize_t sendByte = send(sock, message, 10, 0);

    ASSERT_GT(sendByte, 0) << "Impossible write to socket";

    boost::this_thread::sleep_for(boost::chrono::milliseconds(500));

    ioctl(sock, FIONREAD, &count);

    ASSERT_NE(count, 0) << "No data available on response socket";

    valread = read(sock, buffer, 150);

    ASSERT_EQ(valread, 16) << "Expected 16 bytes, but " << valread << " bytes returned";

    EXPECT_EQ(ntohll(*((uint64_t*)buffer)), 1500 + 1345 + 1200);
    EXPECT_EQ(ntohll(*((uint64_t*)(buffer + sizeof(uint64_t)))), (1500 + 1345 + 1200) / TEST_DURATION);

    close(sock);

    performanceServerInstance->Stop();

    threadServerPerformance->interrupt();
}
