/*
 * MessageDecapsulator.unit.cpp
 *
 *  Created on: May 9, 2019
 *      Author: mbortolon
 */

#include "gtest/gtest.h"

#include "src/server/dataManagerServer/MessageDecapsulator.hpp"

#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"

#include <string>
#include <vector>

#define APPLICATION_ID "aaaaaaa"
#define PHONE_ID "afwe45821sqfr1487cd"
#define INIT_BYTES ""

#define PHONE_ID_2 "fre47s2w5xre6sq35e9"

#define ADDITIONAL_DATA "additional_data"

#include "src/common/dataManager/protocol/InitMessage.pb.h"
#include "src/common/dataManager/protocol/MainMessage.pb.h"
#include "src/common/dataManager/protocol/DataMessage.pb.h"
#include "src/common/dataManager/protocol/EndMessage.pb.h"

TEST(MessageDecapsulator, OneHostSessionCorrect) {
	// Init message decapsulator
	dataManagerServer::MessageDecapsulator* decapsulator = new dataManagerServer::MessageDecapsulator();

	// **** Create init packet ***
	dataManager::protocol::InitMessage initMessage = dataManager::protocol::InitMessage();
	initMessage.set_applicationid(APPLICATION_ID);
	initMessage.set_phoneid(PHONE_ID);
	initMessage.set_initbytes(INIT_BYTES);
	initMessage.set_contentcodec(dataManager::protocol::InitMessage_ContentCodec::InitMessage_ContentCodec_MJPEG);
	initMessage.set_clients(1);
	std::string initMessageEncoded = initMessage.SerializeAsString();

	dataManager::protocol::MainMessage mainMessage = dataManager::protocol::MainMessage();
	mainMessage.set_type(dataManager::protocol::MainMessage_Type::MainMessage_Type_INIT);
	mainMessage.set_info(initMessageEncoded);
	std::string encapsulateInitMessage = mainMessage.SerializeAsString();

	dataManagerServer::MessageDecapsulator::Response response = decapsulator->ElaborateMessage(encapsulateInitMessage.c_str(), encapsulateInitMessage.size());

	ASSERT_EQ(response.type, dataManagerServer::MessageDecapsulator::ResponseType::OK_SEND_RESPONSE);

	dataManager::protocol::InitMessageResponse initMessageResponse = dataManager::protocol::InitMessageResponse();
	initMessageResponse.ParseFromArray(response.response->c_str(), response.response->size());

	std::string streamId(initMessageResponse.streamid());
	ASSERT_NE(streamId.length(), 0);

	// Take out the garbage
	initMessage.Clear();
	initMessageEncoded.clear();
	encapsulateInitMessage.clear();
	delete response.response;

	mainMessage.Clear();

	EXPECT_EQ(streamId.length(), 10);

	// *** Create data packet ***

	// Image encoding
	cv::Mat decodedImage = cv::Mat::zeros(640, 480, CV_8UC3);
	std::vector<uchar> encodedImage = std::vector<uchar>();
	cv::imencode(".jpg", decodedImage, encodedImage);

	// Prepare data message
	dataManager::protocol::DataMessage dataMessage = dataManager::protocol::DataMessage();
	dataMessage.set_frameid(0);
	dataMessage.set_additionaldata(ADDITIONAL_DATA);
	dataMessage.set_data(&encodedImage[0], encodedImage.size());
	std::string dataMessageEncoded = dataMessage.SerializeAsString();

	mainMessage.set_type(dataManager::protocol::MainMessage_Type::MainMessage_Type_DATA);
	mainMessage.set_info(dataMessageEncoded);

	std::string encapsulateDataMessage = mainMessage.SerializeAsString();

	response = decapsulator->ElaborateMessage(encapsulateDataMessage.c_str(), encapsulateDataMessage.size());

	ASSERT_EQ(response.type, dataManagerServer::MessageDecapsulator::ResponseType::OK_NO_ANSWER);

	// Take out the garbage
	encodedImage.clear();
	decodedImage.release();
	dataMessage.Clear();
	dataMessageEncoded.clear();
	encapsulateDataMessage.clear();

	mainMessage.Clear();

	// **** Create end packet ***

	// Prepare data message
	dataManager::protocol::EndMessage endMessage = dataManager::protocol::EndMessage();
	endMessage.set_totalacquireframes(1);
	std::string endMessageEncoded = endMessage.SerializeAsString();

	mainMessage.set_type(dataManager::protocol::MainMessage_Type::MainMessage_Type_END);
	mainMessage.set_info(endMessageEncoded);

	std::string encapsulateEndMessage = mainMessage.SerializeAsString();

	response = decapsulator->ElaborateMessage(encapsulateEndMessage.c_str(), encapsulateEndMessage.size());

	ASSERT_EQ(response.type, dataManagerServer::MessageDecapsulator::ResponseType::ERROR_CLOSE_SOCKET);

	// Take out the garbage
	endMessage.Clear();
	endMessageEncoded.clear();
	encapsulateEndMessage.clear();

	mainMessage.Clear();

	delete decapsulator;

	// *** Check if files exist ***

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

TEST(MessageDecapsulator,MultipleHostSessionCorrect) {
	// Init message decapsulator
	dataManagerServer::MessageDecapsulator* decapsulator_host = new dataManagerServer::MessageDecapsulator();
	dataManagerServer::MessageDecapsulator* decapsulator_client = new dataManagerServer::MessageDecapsulator();

	// **** Create init packets ***

	// +++ host +++

	dataManager::protocol::InitMessage initMessage = dataManager::protocol::InitMessage();
	initMessage.set_applicationid(APPLICATION_ID);
	initMessage.set_phoneid(PHONE_ID);
	initMessage.set_initbytes(INIT_BYTES);
	initMessage.set_contentcodec(dataManager::protocol::InitMessage_ContentCodec::InitMessage_ContentCodec_MJPEG);
	initMessage.set_clients(2);
	std::string initMessageEncoded = initMessage.SerializeAsString();

	dataManager::protocol::MainMessage mainMessage = dataManager::protocol::MainMessage();
	mainMessage.set_type(dataManager::protocol::MainMessage_Type::MainMessage_Type_INIT);
	mainMessage.set_info(initMessageEncoded);
	std::string encapsulateInitMessage = mainMessage.SerializeAsString();

	dataManagerServer::MessageDecapsulator::Response response = decapsulator_host->ElaborateMessage(encapsulateInitMessage.c_str(), encapsulateInitMessage.size());

	ASSERT_EQ(response.type, dataManagerServer::MessageDecapsulator::ResponseType::OK_SEND_RESPONSE);

	dataManager::protocol::InitMessageResponse initMessageResponse = dataManager::protocol::InitMessageResponse();
	initMessageResponse.ParseFromArray(response.response->c_str(), response.response->size());

	std::string streamId(initMessageResponse.streamid());
	ASSERT_NE(streamId.length(), 0);

	// Take out the garbage
	initMessage.Clear();
	initMessageEncoded.clear();
	encapsulateInitMessage.clear();
	delete response.response;

	mainMessage.Clear();

	EXPECT_EQ(streamId.length(), 10);

	// +++ client +++

	initMessage.set_applicationid(APPLICATION_ID);
	initMessage.set_phoneid(PHONE_ID_2);
	initMessage.set_initbytes(INIT_BYTES);
	initMessage.set_contentcodec(dataManager::protocol::InitMessage_ContentCodec::InitMessage_ContentCodec_MJPEG);
	std::cout << streamId << std::endl;
	initMessage.set_streamid(streamId);
	initMessage.set_rejoin(false);
	initMessageEncoded = initMessage.SerializeAsString();

	mainMessage.set_type(dataManager::protocol::MainMessage_Type::MainMessage_Type_INIT);
	mainMessage.set_info(initMessageEncoded);
	encapsulateInitMessage = mainMessage.SerializeAsString();

	response = decapsulator_client->ElaborateMessage(encapsulateInitMessage.c_str(), encapsulateInitMessage.size());

	ASSERT_EQ(response.type, dataManagerServer::MessageDecapsulator::ResponseType::OK_NO_ANSWER);

	// Take out the garbage
	initMessage.Clear();
	initMessageEncoded.clear();
	encapsulateInitMessage.clear();

	mainMessage.Clear();

	// *** Create data packet ***

	// Image encoding
	cv::Mat decodedImage = cv::Mat::zeros(640, 480, CV_8UC3);
	std::vector<uchar> encodedImage = std::vector<uchar>();
	cv::imencode(".jpg", decodedImage, encodedImage);

	// +++ host +++

	// Prepare data message
	dataManager::protocol::DataMessage dataMessage = dataManager::protocol::DataMessage();
	dataMessage.set_frameid(0);
	dataMessage.set_additionaldata(ADDITIONAL_DATA);
	dataMessage.set_data(&encodedImage[0], encodedImage.size());
	std::string dataMessageEncoded = dataMessage.SerializeAsString();

	mainMessage.set_type(dataManager::protocol::MainMessage_Type::MainMessage_Type_DATA);
	mainMessage.set_info(dataMessageEncoded);

	std::string encapsulateDataMessage = mainMessage.SerializeAsString();

	response = decapsulator_host->ElaborateMessage(encapsulateDataMessage.c_str(), encapsulateDataMessage.size());

	ASSERT_EQ(response.type, dataManagerServer::MessageDecapsulator::ResponseType::OK_NO_ANSWER);

	// Take out the garbage
	decodedImage.release();
	dataMessage.Clear();
	dataMessageEncoded.clear();
	encapsulateDataMessage.clear();

	mainMessage.Clear();

	// +++ client +++

	// Prepare data message
	dataMessage = dataManager::protocol::DataMessage();
	dataMessage.set_frameid(0);
	dataMessage.set_additionaldata(ADDITIONAL_DATA);
	dataMessage.set_data(&encodedImage[0], encodedImage.size());
	dataMessageEncoded = dataMessage.SerializeAsString();

	mainMessage.set_type(dataManager::protocol::MainMessage_Type::MainMessage_Type_DATA);
	mainMessage.set_info(dataMessageEncoded);

	encapsulateDataMessage = mainMessage.SerializeAsString();

	response = decapsulator_client->ElaborateMessage(encapsulateDataMessage.c_str(), encapsulateDataMessage.size());

	ASSERT_EQ(response.type, dataManagerServer::MessageDecapsulator::ResponseType::OK_NO_ANSWER);

	// Take out the garbage
	encodedImage.clear();
	decodedImage.release();
	dataMessage.Clear();
	dataMessageEncoded.clear();
	encapsulateDataMessage.clear();

	mainMessage.Clear();

	// **** Create end packet ***

	// +++ host +++

	// Prepare data message
	dataManager::protocol::EndMessage endMessage = dataManager::protocol::EndMessage();
	endMessage.set_totalacquireframes(1);
	std::string endMessageEncoded = endMessage.SerializeAsString();

	mainMessage.set_type(dataManager::protocol::MainMessage_Type::MainMessage_Type_END);
	mainMessage.set_info(endMessageEncoded);

	std::string encapsulateEndMessage = mainMessage.SerializeAsString();

	response = decapsulator_host->ElaborateMessage(encapsulateEndMessage.c_str(), encapsulateEndMessage.size());

	ASSERT_EQ(response.type, dataManagerServer::MessageDecapsulator::ResponseType::ERROR_CLOSE_SOCKET);

	// Take out the garbage
	endMessage.Clear();
	endMessageEncoded.clear();
	encapsulateEndMessage.clear();

	mainMessage.Clear();

	delete decapsulator_host;
	delete decapsulator_client;

	// *** Check if files exist ***
	std::ostringstream resultFolderStream;
	resultFolderStream << "/tmp/uploadedImages/" << streamId << "/";
	std::string resultFolder = resultFolderStream.str();

	std::ostringstream phoneOneResultFolderStream;
	phoneOneResultFolderStream << resultFolder << PHONE_ID << "/";
	std::string phoneOneResultFolder = phoneOneResultFolderStream.str();

	std::ostringstream phoneTwoResultFolderStream;
	phoneTwoResultFolderStream << resultFolder << PHONE_ID_2 << "/";
	std::string phoneTwoResultFolder = phoneTwoResultFolderStream.str();

	std::ostringstream imgOneResultFilepathStream;
	imgOneResultFilepathStream << phoneOneResultFolder << "00000.jpg";
	std::string imgOneResultFilepath = imgOneResultFilepathStream.str();

	std::ostringstream imgTwoResultFilepathStream;
	imgTwoResultFilepathStream << phoneTwoResultFolder << "00000.jpg";
	std::string imgTwoResultFilepath = imgTwoResultFilepathStream.str();

	std::ostringstream binOneResultFilepathStream;
	binOneResultFilepathStream << phoneOneResultFolder << "00000.bin";
	std::string binOneResultFilepath = binOneResultFilepathStream.str();

	std::ostringstream binTwoResultFilepathStream;
	binTwoResultFilepathStream << phoneTwoResultFolder << "00000.bin";
	std::string binTwoResultFilepath = binTwoResultFilepathStream.str();

	EXPECT_TRUE(boost::filesystem::exists(imgOneResultFilepath));
	EXPECT_TRUE(boost::filesystem::exists(imgTwoResultFilepath));
	EXPECT_TRUE(boost::filesystem::exists(binOneResultFilepath));
	EXPECT_TRUE(boost::filesystem::exists(binTwoResultFilepath));

	boost::filesystem::remove_all(resultFolder);
}

TEST(MessageDecapsulator,MultipleHostWithHostRejoinSessionCorrect) {
	// Init message decapsulator
	dataManagerServer::MessageDecapsulator* decapsulator_host_first = new dataManagerServer::MessageDecapsulator();
	dataManagerServer::MessageDecapsulator* decapsulator_client = new dataManagerServer::MessageDecapsulator();

	// **** Create init packets ***

	// +++ host +++

	dataManager::protocol::InitMessage initMessage = dataManager::protocol::InitMessage();
	initMessage.set_applicationid(APPLICATION_ID);
	initMessage.set_phoneid(PHONE_ID);
	initMessage.set_initbytes(INIT_BYTES);
	initMessage.set_contentcodec(dataManager::protocol::InitMessage_ContentCodec::InitMessage_ContentCodec_MJPEG);
	initMessage.set_clients(2);
	std::string initMessageEncoded = initMessage.SerializeAsString();

	dataManager::protocol::MainMessage mainMessage = dataManager::protocol::MainMessage();
	mainMessage.set_type(dataManager::protocol::MainMessage_Type::MainMessage_Type_INIT);
	mainMessage.set_info(initMessageEncoded);
	std::string encapsulateInitMessage = mainMessage.SerializeAsString();

	dataManagerServer::MessageDecapsulator::Response response = decapsulator_host_first->ElaborateMessage(encapsulateInitMessage.c_str(), encapsulateInitMessage.size());

	ASSERT_EQ(response.type, dataManagerServer::MessageDecapsulator::ResponseType::OK_SEND_RESPONSE);

	dataManager::protocol::InitMessageResponse initMessageResponse = dataManager::protocol::InitMessageResponse();
	initMessageResponse.ParseFromArray(response.response->c_str(), response.response->size());

	std::string streamId(initMessageResponse.streamid());
	ASSERT_NE(streamId.length(), 0);

	// Take out the garbage
	initMessage.Clear();
	initMessageEncoded.clear();
	encapsulateInitMessage.clear();
	delete response.response;

	mainMessage.Clear();

	EXPECT_EQ(streamId.length(), 10);

	// +++ client +++

	initMessage.set_applicationid(APPLICATION_ID);
	initMessage.set_phoneid(PHONE_ID_2);
	initMessage.set_initbytes(INIT_BYTES);
	initMessage.set_contentcodec(dataManager::protocol::InitMessage_ContentCodec::InitMessage_ContentCodec_MJPEG);
	std::cout << streamId << std::endl;
	initMessage.set_streamid(streamId);
	initMessage.set_rejoin(false);
	initMessageEncoded = initMessage.SerializeAsString();

	mainMessage.set_type(dataManager::protocol::MainMessage_Type::MainMessage_Type_INIT);
	mainMessage.set_info(initMessageEncoded);
	encapsulateInitMessage = mainMessage.SerializeAsString();

	response = decapsulator_client->ElaborateMessage(encapsulateInitMessage.c_str(), encapsulateInitMessage.size());

	ASSERT_EQ(response.type, dataManagerServer::MessageDecapsulator::ResponseType::OK_NO_ANSWER);

	// Take out the garbage
	initMessage.Clear();
	initMessageEncoded.clear();
	encapsulateInitMessage.clear();

	mainMessage.Clear();

	// *** Create data packet ***

	// Image encoding
	cv::Mat decodedImage = cv::Mat::zeros(640, 480, CV_8UC3);
	std::vector<uchar> encodedImage = std::vector<uchar>();
	cv::imencode(".jpg", decodedImage, encodedImage);

	// +++ host +++

	// Prepare data message
	dataManager::protocol::DataMessage dataMessage = dataManager::protocol::DataMessage();
	dataMessage.set_frameid(0);
	dataMessage.set_additionaldata(ADDITIONAL_DATA);
	dataMessage.set_data(&encodedImage[0], encodedImage.size());
	std::string dataMessageEncoded = dataMessage.SerializeAsString();

	mainMessage.set_type(dataManager::protocol::MainMessage_Type::MainMessage_Type_DATA);
	mainMessage.set_info(dataMessageEncoded);

	std::string encapsulateDataMessage = mainMessage.SerializeAsString();

	response = decapsulator_host_first->ElaborateMessage(encapsulateDataMessage.c_str(), encapsulateDataMessage.size());

	ASSERT_EQ(response.type, dataManagerServer::MessageDecapsulator::ResponseType::OK_NO_ANSWER);

	// Take out the garbage
	decodedImage.release();
	dataMessage.Clear();
	dataMessageEncoded.clear();
	encapsulateDataMessage.clear();

	mainMessage.Clear();

	// +++ client +++

	// Prepare data message
	dataMessage = dataManager::protocol::DataMessage();
	dataMessage.set_frameid(0);
	dataMessage.set_additionaldata(ADDITIONAL_DATA);
	dataMessage.set_data(&encodedImage[0], encodedImage.size());
	dataMessageEncoded = dataMessage.SerializeAsString();

	mainMessage.set_type(dataManager::protocol::MainMessage_Type::MainMessage_Type_DATA);
	mainMessage.set_info(dataMessageEncoded);

	encapsulateDataMessage = mainMessage.SerializeAsString();

	response = decapsulator_client->ElaborateMessage(encapsulateDataMessage.c_str(), encapsulateDataMessage.size());

	ASSERT_EQ(response.type, dataManagerServer::MessageDecapsulator::ResponseType::OK_NO_ANSWER);

	// Take out the garbage
	encodedImage.clear();
	decodedImage.release();
	dataMessage.Clear();
	dataMessageEncoded.clear();
	encapsulateDataMessage.clear();

	mainMessage.Clear();

	// +++ host +++

	// create parallel connection
	delete decapsulator_host_first;

	dataManagerServer::MessageDecapsulator* decapsulator_host_second = new dataManagerServer::MessageDecapsulator();
	initMessage = dataManager::protocol::InitMessage();
	initMessage.set_applicationid(APPLICATION_ID);
	initMessage.set_phoneid(PHONE_ID);
	initMessage.set_initbytes(INIT_BYTES);
	initMessage.set_contentcodec(dataManager::protocol::InitMessage_ContentCodec::InitMessage_ContentCodec_MJPEG);
	initMessage.set_streamid(streamId);
	initMessage.set_rejoin(true);
	initMessageEncoded = initMessage.SerializeAsString();

	mainMessage.set_type(dataManager::protocol::MainMessage_Type::MainMessage_Type_INIT);
	mainMessage.set_info(initMessageEncoded);
	encapsulateInitMessage = mainMessage.SerializeAsString();

	response = decapsulator_host_second->ElaborateMessage(encapsulateInitMessage.c_str(), encapsulateInitMessage.size());

	ASSERT_EQ(response.type, dataManagerServer::MessageDecapsulator::ResponseType::OK_NO_ANSWER);

	// Take out the garbage
	initMessage.Clear();
	initMessageEncoded.clear();
	encapsulateInitMessage.clear();

	mainMessage.Clear();

	// **** Create end packet ***

	// +++ host +++

	// Prepare data message
	dataManager::protocol::EndMessage endMessage = dataManager::protocol::EndMessage();
	endMessage.set_totalacquireframes(1);
	std::string endMessageEncoded = endMessage.SerializeAsString();

	mainMessage.set_type(dataManager::protocol::MainMessage_Type::MainMessage_Type_END);
	mainMessage.set_info(endMessageEncoded);

	std::string encapsulateEndMessage = mainMessage.SerializeAsString();

	response = decapsulator_host_second->ElaborateMessage(encapsulateEndMessage.c_str(), encapsulateEndMessage.size());

	ASSERT_EQ(response.type, dataManagerServer::MessageDecapsulator::ResponseType::ERROR_CLOSE_SOCKET);

	// Take out the garbage
	endMessage.Clear();
	endMessageEncoded.clear();
	encapsulateEndMessage.clear();

	mainMessage.Clear();

	delete decapsulator_host_second;
	delete decapsulator_client;

	// *** Check if files exist ***
	std::ostringstream resultFolderStream;
	resultFolderStream << "/tmp/uploadedImages/" << streamId << "/";
	std::string resultFolder = resultFolderStream.str();

	std::ostringstream phoneOneResultFolderStream;
	phoneOneResultFolderStream << resultFolder << PHONE_ID << "/";
	std::string phoneOneResultFolder = phoneOneResultFolderStream.str();

	std::ostringstream phoneTwoResultFolderStream;
	phoneTwoResultFolderStream << resultFolder << PHONE_ID_2 << "/";
	std::string phoneTwoResultFolder = phoneTwoResultFolderStream.str();

	std::ostringstream imgOneResultFilepathStream;
	imgOneResultFilepathStream << phoneOneResultFolder << "00000.jpg";
	std::string imgOneResultFilepath = imgOneResultFilepathStream.str();

	std::ostringstream imgTwoResultFilepathStream;
	imgTwoResultFilepathStream << phoneTwoResultFolder << "00000.jpg";
	std::string imgTwoResultFilepath = imgTwoResultFilepathStream.str();

	std::ostringstream binOneResultFilepathStream;
	binOneResultFilepathStream << phoneOneResultFolder << "00000.bin";
	std::string binOneResultFilepath = binOneResultFilepathStream.str();

	std::ostringstream binTwoResultFilepathStream;
	binTwoResultFilepathStream << phoneTwoResultFolder << "00000.bin";
	std::string binTwoResultFilepath = binTwoResultFilepathStream.str();

	EXPECT_TRUE(boost::filesystem::exists(imgOneResultFilepath));
	EXPECT_TRUE(boost::filesystem::exists(imgTwoResultFilepath));
	EXPECT_TRUE(boost::filesystem::exists(binOneResultFilepath));
	EXPECT_TRUE(boost::filesystem::exists(binTwoResultFilepath));

	boost::filesystem::remove_all(resultFolder);
}
