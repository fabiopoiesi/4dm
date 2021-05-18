#include "gtest/gtest.h"

#include "boost/chrono.hpp"
#include "boost/thread/thread.hpp"
#include "boost/asio.hpp"
#include "src/server/performanceServer/PerformanceMeasureTCPServer.hpp"

#include "src/lib/performanceTesting.h"

#define PORT 5959
#define TEST_DURATION 2
#define IP_DURATION "127.0.0.1"

performanceServer::PerformanceMeasureTCPServer* performanceServerInstance;
bool serverRun = false;

int performanceServerThreadFunction() {
	boost::asio::io_context io_context;
	performanceServerInstance = new performanceServer::PerformanceMeasureTCPServer(io_context, TEST_DURATION, PORT);
	serverRun = true;
    io_context.run();
	return 0;
}

TEST(PerformanceTestingLib, TestCorrectness){
	boost::thread* threadServerPerformance = new boost::thread(&performanceServerThreadFunction);

	// Wait for server is ready
	int maxTry = 0;
	while(maxTry >= 0 && serverRun == false) {
		boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
		maxTry--;
	}
	ASSERT_TRUE(serverRun) << "Thread not start in time";

	// Open connection to the server
	Performance::performanceTesting(IP_DURATION, PORT);

	EXPECT_TRUE(Performance::getStatus());


	if(Performance::getStatus() == true) {
		char* errorMessage = Performance::getErrorMessage();
		EXPECT_EQ(errorMessage, nullptr);
		EXPECT_GT(Performance::getAvgRate(), 0);
		EXPECT_GT(Performance::getTotalByteSend(), 0);
	} else {
		char* errorMessage = Performance::getErrorMessage();
		std::cout << errorMessage << std::endl;
	}

	performanceServerInstance->Stop();

	threadServerPerformance->interrupt();

	delete threadServerPerformance;
}
