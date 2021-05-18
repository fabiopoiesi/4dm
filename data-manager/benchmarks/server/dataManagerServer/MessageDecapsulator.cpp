/*
 * MessageDecapsulator.cpp
 *
 *  Created on: May 10, 2019
 *      Author: mbortolon
 */

#include "benchmark/benchmark.h"

#include "src/server/dataManagerServer/MessageDecapsulator.hpp"

#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"

#include "src/common/dataManager/protocol/InitMessage.pb.h"
#include "src/common/dataManager/protocol/MainMessage.pb.h"
#include "src/common/dataManager/protocol/DataMessage.pb.h"
#include "src/common/dataManager/protocol/EndMessage.pb.h"

#include <string>
#include <vector>

#define APPLICATION_ID "aaaaaaa"
#define PHONE_ID "afwe45821sqfr1487cd"
#define INIT_BYTES ""

#define PHONE_ID_2 "fre47s2w5xre6sq35e9"

#define ADDITIONAL_DATA "additional_data"

static void createInitMessage(dataManager::protocol::InitMessage& initMessage) {
	initMessage.set_applicationid(APPLICATION_ID);
	initMessage.set_phoneid(PHONE_ID);
	initMessage.set_initbytes(INIT_BYTES);
	initMessage.set_contentcodec(dataManager::protocol::InitMessage_ContentCodec::InitMessage_ContentCodec_MJPEG);
}

static void createInitMessageClient(dataManager::protocol::InitMessage& initMessage, std::string& streamId) {
	createInitMessage(initMessage);
	initMessage.set_streamid(streamId);
}

static void createInitMessageHost(dataManager::protocol::InitMessage& initMessage, u_int32_t numberOfClient) {
	createInitMessage(initMessage);
	initMessage.set_clients(numberOfClient);
}

static std::string encapsulateInsideMainMessage(std::string& message, dataManager::protocol::MainMessage_Type messageType) {
	dataManager::protocol::MainMessage mainMessage = dataManager::protocol::MainMessage();
	mainMessage.set_type(messageType);
	mainMessage.set_info(message);
	return mainMessage.SerializeAsString();
}

static void BM_MessageDecapsulator(benchmark::State& state) {
	u_int32_t numberOfSender = 1;
	// Init message decapsulator
	dataManagerServer::MessageDecapsulator* decapsulatorHost = new dataManagerServer::MessageDecapsulator();
	dataManagerServer::MessageDecapsulator** decapsulatorList = new dataManagerServer::MessageDecapsulator*[numberOfSender];

	decapsulatorList[0] = decapsulatorHost;
	for(u_int32_t i = 1; i < numberOfSender; i++) {
		decapsulatorList[i] = new dataManagerServer::MessageDecapsulator();
	}

	// **** Create init packets ***

	// +++ host +++

	dataManager::protocol::InitMessage initMessage = dataManager::protocol::InitMessage();
	createInitMessageHost(initMessage, numberOfSender);
	std::string initMessageEncoded = initMessage.SerializeAsString();

	std::string encapsulateInitMessage = encapsulateInsideMainMessage(initMessageEncoded, dataManager::protocol::MainMessage_Type::MainMessage_Type_INIT);

	dataManagerServer::MessageDecapsulator::Response response = decapsulatorHost->ElaborateMessage(encapsulateInitMessage.c_str(), encapsulateInitMessage.size());

	dataManager::protocol::InitMessageResponse initMessageResponse = dataManager::protocol::InitMessageResponse();
	initMessageResponse.ParseFromArray(response.response->c_str(), response.response->size());

	std::string streamId(initMessageResponse.streamid());
	
	// Take out the garbage
	initMessage.Clear();
	initMessageEncoded.clear();
	encapsulateInitMessage.clear();
	delete response.response;

	// +++ client +++

	for(u_int32_t i = 1; i < numberOfSender; i++) {
		createInitMessageClient(initMessage, streamId);
		initMessageEncoded = initMessage.SerializeAsString();
		encapsulateInitMessage = encapsulateInsideMainMessage(initMessageEncoded, dataManager::protocol::MainMessage_Type::MainMessage_Type_INIT);

		response = decapsulatorList[i]->ElaborateMessage(encapsulateInitMessage.c_str(), encapsulateInitMessage.size());

	}

	// Take out the garbage
	initMessage.Clear();
	initMessageEncoded.clear();
	encapsulateInitMessage.clear();

	// *** Test data packet sending performance ***

	// Image encoding
	cv::Mat decodedImage = cv::Mat::zeros(640, 480, CV_8UC3);
	std::vector<uchar> encodedImage = std::vector<uchar>();
	cv::imencode(".jpg", decodedImage, encodedImage);

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
			decapsulatorList[i]->ElaborateMessage(encapsulateDataMessage.c_str(), encapsulateDataMessage.size());
		}
		frameId++;
		encapsulateDataMessage.clear();
		dataMessageEncoded.clear();
	}

	dataMessage.Clear();
	encodedImage.clear();

	// **** Create end packet ***

	// Prepare data message
	dataManager::protocol::EndMessage endMessage = dataManager::protocol::EndMessage();
	endMessage.set_totalacquireframes(frameId + 1);
	std::string endMessageEncoded = endMessage.SerializeAsString();

	std::string encapsulateEndMessage = encapsulateInsideMainMessage(endMessageEncoded, dataManager::protocol::MainMessage_Type::MainMessage_Type_END);

	response = decapsulatorHost->ElaborateMessage(encapsulateEndMessage.c_str(), encapsulateEndMessage.size());

	encapsulateEndMessage.clear();
	endMessageEncoded.clear();
	endMessage.Clear();

	for(u_int32_t i = 0; i < numberOfSender; i++) {
		delete decapsulatorList[i];
	}

	// *** Check if files exist ***
	std::ostringstream resultFolderStream;
	resultFolderStream << "/tmp/uploadedImages/" << streamId << "/";
	std::string resultFolder = resultFolderStream.str();

	boost::filesystem::remove_all(resultFolder);

	state.SetItemsProcessed(state.iterations());
}

BENCHMARK(BM_MessageDecapsulator)->Unit(benchmark::kMillisecond);
