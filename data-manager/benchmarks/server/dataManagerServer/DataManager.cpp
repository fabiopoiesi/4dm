/*
 * DataManager.cpp
 *
 *  Created on: May 12, 2019
 *      Author: matteo
 */

#include "benchmark/benchmark.h"

#include <string>
#include <vector>

#include <iostream>

#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

#include "boost/chrono.hpp"
#include "boost/thread/thread.hpp"
#include "boost/filesystem.hpp"
#include "boost/dll.hpp"

#include "src/server/dataManagerServer/DataManagerTCPServer.hpp"

#include "src/common/dataManager/protocol/InitMessage.pb.h"
#include "src/common/dataManager/protocol/MainMessage.pb.h"
#include "src/common/dataManager/protocol/DataMessage.pb.h"
#include "src/common/dataManager/protocol/EndMessage.pb.h"

#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"

#define PORT 5960

#define APPLICATION_ID "aaaaaaa"
#define HOST_PHONE_ID "afwe45821sqfr1487cd"
#define PHONE_ID_LENGTH 13
#define INIT_BYTES ""

#define ADDITIONAL_DATA "additional_data"

#define PACKET_MAX_SIZE 1200

#define EXAMPLE_IMAGE_PATH "data_manager_benchmarks.runfiles/__main__/benchmarks/server/dataManagerServer/example-image.jpg"

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

static void createInitMessage(dataManager::protocol::InitMessage& initMessage, std::string& phoneId) {
	initMessage.set_applicationid(APPLICATION_ID);
	initMessage.set_phoneid(phoneId);
	initMessage.set_initbytes(INIT_BYTES);
	initMessage.set_contentcodec(dataManager::protocol::InitMessage_ContentCodec::InitMessage_ContentCodec_MJPEG);
}

static void createInitMessageClient(dataManager::protocol::InitMessage& initMessage, std::string& phoneId, std::string& streamId) {
	createInitMessage(initMessage, phoneId);
	initMessage.set_streamid(streamId);
}

static void createInitMessageHost(dataManager::protocol::InitMessage& initMessage, std::string& phoneId, u_int32_t numberOfClient) {
	createInitMessage(initMessage, phoneId);
	initMessage.set_clients(numberOfClient);
}

static std::string encapsulateInsideMainMessage(std::string& message, dataManager::protocol::MainMessage_Type messageType) {
	dataManager::protocol::MainMessage mainMessage = dataManager::protocol::MainMessage();
	mainMessage.set_type(messageType);
	mainMessage.set_info(message);
	return mainMessage.SerializeAsString();
}

static std::string randomPhoneId() {
	std::string phoneId(PHONE_ID_LENGTH, 'a');
	for(unsigned int i = 0; i < PHONE_ID_LENGTH; i++) {
		int randomChar = rand()%(26+10);
		if(randomChar < 26) {
			phoneId[i] = 'a' + randomChar;
		} else {
			phoneId[i] = '0' + randomChar - 26;
		}
	}
	return phoneId;
}

static void BM_DataManagerServer(benchmark::State& state) {
	boost::thread* threadServerPerformance = new boost::thread(
			&dataManagerServerThreadFunction);

	u_int32_t numberOfSender = 1;
	bool realImage = true;

	struct sockaddr_in address;

	int* sockets = new int[numberOfSender];

	struct sockaddr_in serv_addr;
	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	// Convert IPv4 and IPv6 addresses from text to binary form
	inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

	// Wait for connection ready
	int maxTry = 0;
	while (maxTry >= 0 && serverRun == false) {
		boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
		maxTry--;
	}

	std::cout << "Start connection" << std::endl;
	for(u_int32_t i = 0; i < numberOfSender; i++) {
		sockets[i] = socket(AF_INET, SOCK_STREAM, 0);
		connect(sockets[i], (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	}

	// +++ host +++

	dataManager::protocol::InitMessage initMessage = dataManager::protocol::InitMessage();
	std::string hostId(HOST_PHONE_ID);
	createInitMessageHost(initMessage, hostId, numberOfSender);
	std::string initMessageEncoded = initMessage.SerializeAsString();

	std::string encapsulateInitMessage = encapsulateInsideMainMessage(initMessageEncoded, dataManager::protocol::MainMessage_Type::MainMessage_Type_INIT);

	socketSend(sockets[0], encapsulateInitMessage.c_str(), encapsulateInitMessage.size());

	int count = 0;
	int numberTry = 0;
	while (numberTry < 20 && count == 0) {
		ioctl(sockets[0], FIONREAD, &count);
		numberTry++;
		boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
	}

	char buffer[400] = { 0 };
	read(sockets[0], buffer, 400);

	dataManager::protocol::InitMessageResponse initMessageResponse =
			dataManager::protocol::InitMessageResponse();
	initMessageResponse.ParseFromArray(buffer, count);

	std::string streamId(initMessageResponse.streamid());

	// Take out the garbage
	initMessage.Clear();
	initMessageEncoded.clear();
	encapsulateInitMessage.clear();
	initMessageResponse.Clear();

	// Init other client
	std::cout << "Init connection" << std::endl;
	for(u_int32_t i = 1; i < numberOfSender; i++) {
		dataManager::protocol::InitMessage initMessage = dataManager::protocol::InitMessage();
		std::string phoneId = randomPhoneId();
		createInitMessageClient(initMessage, phoneId, streamId);
		initMessageEncoded = initMessage.SerializeAsString();
		encapsulateInitMessage = encapsulateInsideMainMessage(initMessageEncoded, dataManager::protocol::MainMessage_Type::MainMessage_Type_INIT);

		socketSend(sockets[i], encapsulateInitMessage.c_str(), encapsulateInitMessage.size());

		initMessage.Clear();
		initMessageEncoded.clear();
		encapsulateInitMessage.clear();
	}

	
	std::cout << "Image path: " << boost::dll::program_location().parent_path() / EXAMPLE_IMAGE_PATH << std::endl;
	// Image encoding
	cv::Mat decodedImage;
	if(realImage) {
		boost::filesystem::path path = boost::dll::program_location().parent_path() / EXAMPLE_IMAGE_PATH;
		decodedImage = cv::imread(path.string(), cv::IMREAD_COLOR);
	} else {
		decodedImage = cv::Mat::zeros(640, 480, CV_8UC3);
	}

	std::vector<uchar> encodedImage = std::vector<uchar>();
	cv::imencode(".jpg", decodedImage, encodedImage);

	// generation garbage
	decodedImage.release();

	// Prepare data message
	dataManager::protocol::DataMessage dataMessage = dataManager::protocol::DataMessage();
	dataMessage.set_additionaldata(ADDITIONAL_DATA);
	dataMessage.set_data(&encodedImage[0], encodedImage.size());
	u_int32_t frameId = 0;

	for(auto _ : state) {
		dataMessage.set_frameid(frameId);
		std::string dataMessageEncoded = dataMessage.SerializeAsString();
		std::string encapsulateDataMessage = encapsulateInsideMainMessage(dataMessageEncoded, dataManager::protocol::MainMessage_Type::MainMessage_Type_DATA);

		for(u_int32_t i = 0; i < numberOfSender; i++) {
			socketSend(sockets[i], encapsulateDataMessage.c_str(), encapsulateDataMessage.size());
		}
		frameId++;
		encapsulateDataMessage.clear();
		dataMessageEncoded.clear();
	}

	// Take out the garbage
	dataMessage.Clear();
	encodedImage.clear();

	// Prepare end message
	dataManager::protocol::EndMessage endMessage = dataManager::protocol::EndMessage();
	endMessage.set_totalacquireframes(frameId + 1);
	std::string endMessageEncoded = endMessage.SerializeAsString();

	std::string encapsulateEndMessage = encapsulateInsideMainMessage(endMessageEncoded, dataManager::protocol::MainMessage_Type::MainMessage_Type_END);

	socketSend(sockets[0], encapsulateEndMessage.c_str(), encapsulateEndMessage.size());

	encapsulateEndMessage.clear();
	endMessageEncoded.clear();
	endMessage.Clear();

	for(u_int32_t i = 0; i < numberOfSender; i++) {
		close(sockets[i]);
	}

	dataManagerServerInstance->Stop();

	threadServerPerformance->interrupt();
	serverRun = false;

	// Remove all file
	std::ostringstream resultFolderStream;
	resultFolderStream << "/tmp/uploadedImages/" << streamId << "/";
	std::string resultFolder = resultFolderStream.str();

	boost::filesystem::remove_all(resultFolder);

	state.SetItemsProcessed(state.iterations());
}

BENCHMARK(BM_DataManagerServer)->Unit(benchmark::kMillisecond);
