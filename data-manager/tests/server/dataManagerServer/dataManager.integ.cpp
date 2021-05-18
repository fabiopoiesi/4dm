/*
 * dataManager.integ.cpp
 *
 *  Created on: May 9, 2019
 *      Author: mbortolon
 */

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
#include "src/server/dataManagerServer/DataManagerTCPServer.hpp"

#include "src/common/dataManager/protocol/InitMessage.pb.h"
#include "src/common/dataManager/protocol/MainMessage.pb.h"
#include "src/common/dataManager/protocol/DataMessage.pb.h"
#include "src/common/dataManager/protocol/EndMessage.pb.h"

#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"

#define PORT 5960

#define APPLICATION_ID "aaaaaaa"
#define PHONE_ID "afwe45821sqfr1487cd"
#define INIT_BYTES ""

#define ADDITIONAL_DATA "additional_data"

#define PACKET_MAX_SIZE 1200

dataManagerServer::DataManagerTCPServer* dataManagerServerInstance;
bool serverRun = false;

int dataManagerServerThreadFunction() {
	boost::asio::io_context io_context;
	dataManagerServerInstance = new dataManagerServer::DataManagerTCPServer(
			io_context, PORT);
	serverRun = true;
	io_context.run();
	return 0;
}

char firstPacketBuffer[PACKET_MAX_SIZE] = { 0 };

bool socketSend(int sock, const char* array, size_t size) {
	uint32_t size_net = htonl((uint32_t) size);
	std::memcpy(firstPacketBuffer, &size_net, sizeof(uint32_t));
	unsigned long int byteToSend = std::min<unsigned long int>(
			PACKET_MAX_SIZE - sizeof(uint32_t), size);
	std::memcpy(&firstPacketBuffer[sizeof(uint32_t)], array, byteToSend);
	size_t sendBytes = send(sock, firstPacketBuffer,
			byteToSend + sizeof(uint32_t), 0);
	size_t pointer = byteToSend;
	if (sendBytes <= 0) {
		return false;
	}

	while (pointer < size) {
		byteToSend = std::min<unsigned long int>(PACKET_MAX_SIZE,
				size - pointer);
		size_t sendBytes = send(sock, &array[pointer], byteToSend, 0);
		pointer += byteToSend;
		if (sendBytes <= 0) {
			return false;
		}
	}
	return true;
}

TEST(DataManagerServer, TestDataManagerCorrectness) {
	boost::thread* threadServerPerformance = new boost::thread(
			&dataManagerServerThreadFunction);

	struct sockaddr_in address;
	int sock = 0;
	struct sockaddr_in serv_addr;

	char buffer[400] = { 0 };

	ASSERT_GE(sock = socket(AF_INET, SOCK_STREAM, 0), 0)<< "Socket creation error";

	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	// Convert IPv4 and IPv6 addresses from text to binary form
	ASSERT_GT(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr), 0)<< "Invalid address/ Address not supported";

	// Wait for connection ready
	int maxTry = 0;
	while (maxTry >= 0 && serverRun == false) {
		boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
		maxTry--;
	}

	ASSERT_TRUE(serverRun)<< "Thread not start in time";

	std::cout << "Start connection" << std::endl;
	ASSERT_GE(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)), 0)<< "Connection Failed";

	// Prepare initialization message
	dataManager::protocol::InitMessage initMessage =
			dataManager::protocol::InitMessage();
	initMessage.set_applicationid(APPLICATION_ID);
	initMessage.set_phoneid(PHONE_ID);
	initMessage.set_initbytes(INIT_BYTES);
	initMessage.set_contentcodec(
			dataManager::protocol::InitMessage_ContentCodec::InitMessage_ContentCodec_MJPEG);
	initMessage.set_clients(1);
	std::string initMessageEncoded = initMessage.SerializeAsString();

	dataManager::protocol::MainMessage mainMessage =
			dataManager::protocol::MainMessage();
	mainMessage.set_type(
			dataManager::protocol::MainMessage_Type::MainMessage_Type_INIT);
	mainMessage.set_info(initMessageEncoded);
	std::string encapsulateInitMessage = mainMessage.SerializeAsString();
	std::cout << std::endl;

	bool result = socketSend(sock, encapsulateInitMessage.c_str(),
			encapsulateInitMessage.size());

	ASSERT_TRUE(result);

	int count = 0;

	int numberTry = 0;
	while (numberTry < 20 && count == 0) {
		ioctl(sock, FIONREAD, &count);
		numberTry++;
		boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
	}

	ASSERT_NE(count, 0)<< "No data available on response socket";

	read(sock, buffer, 150);

	dataManager::protocol::InitMessageResponse initMessageResponse =
			dataManager::protocol::InitMessageResponse();
	initMessageResponse.ParseFromArray(buffer + sizeof(uint32_t), count - sizeof(uint32_t));

	std::string streamId = initMessageResponse.streamid();

	// Take out the garbage
	initMessage.Clear();
	initMessageEncoded.clear();
	encapsulateInitMessage.clear();

	mainMessage.Clear();

	// Image encoding
	cv::Mat decodedImage = cv::Mat::zeros(640, 480, CV_8UC3);
	std::vector<uchar> encodedImage = std::vector<uchar>();
	cv::imencode(".jpg", decodedImage, encodedImage);

	// Prepare data message
	dataManager::protocol::DataMessage dataMessage =
			dataManager::protocol::DataMessage();
	dataMessage.set_frameid(0);
	dataMessage.set_additionaldata(ADDITIONAL_DATA);
	dataMessage.set_data(&encodedImage[0], encodedImage.size());
	std::string dataMessageEncoded = dataMessage.SerializeAsString();

	mainMessage.set_type(
			dataManager::protocol::MainMessage_Type::MainMessage_Type_DATA);
	mainMessage.set_info(dataMessageEncoded);

	std::string encapsulateDataMessage = mainMessage.SerializeAsString();

	result = socketSend(sock, encapsulateDataMessage.c_str(),
			encapsulateDataMessage.size());

	ASSERT_TRUE(result);

	// Take out the garbage
	encodedImage.clear();
	decodedImage.release();
	dataMessage.Clear();
	dataMessageEncoded.clear();
	encapsulateDataMessage.clear();

	mainMessage.Clear();

	// Prepare end message
	dataManager::protocol::EndMessage endMessage =
			dataManager::protocol::EndMessage();
	endMessage.set_totalacquireframes(1);
	std::string endMessageEncoded = endMessage.SerializeAsString();

	mainMessage.set_type(
			dataManager::protocol::MainMessage_Type::MainMessage_Type_END);
	mainMessage.set_info(endMessageEncoded);

	std::string encapsulateEndMessage = mainMessage.SerializeAsString();

	result = socketSend(sock, encapsulateEndMessage.c_str(),
			encapsulateEndMessage.size());

	ASSERT_TRUE(result);

	// Take out the garbage
	endMessage.Clear();
	endMessageEncoded.clear();
	encapsulateEndMessage.clear();

	mainMessage.Clear();

	close(sock);

	dataManagerServerInstance->Stop();

	threadServerPerformance->interrupt();

	// Check if files exist

	std::ostringstream resultFolderStream;
	resultFolderStream << "/tmp/uploadedImages/" << streamId << "/";
	std::string resultFolder = resultFolderStream.str();

	std::ostringstream phoneOneResultFolderStream;
	phoneOneResultFolderStream << resultFolder << PHONE_ID << "/";
	std::string phoneOneResultFolder = phoneOneResultFolderStream.str();

	std::ostringstream imgOneResultFilepathStream;
	imgOneResultFilepathStream << phoneOneResultFolder << "00000.jpg";
	std::string imgOneResultFilepath = imgOneResultFilepathStream.str();

	std::ostringstream binOneResultFilepathStream;
	binOneResultFilepathStream << phoneOneResultFolder << "00000.bin";
	std::string binOneResultFilepath = binOneResultFilepathStream.str();

	EXPECT_TRUE(boost::filesystem::exists(imgOneResultFilepath));
	EXPECT_TRUE(boost::filesystem::exists(binOneResultFilepath));

	boost::filesystem::remove_all(resultFolder);
}
