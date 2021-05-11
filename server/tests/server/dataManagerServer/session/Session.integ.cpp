/*
 * Session.unit.cpp
 *
 *  Created on: May 6, 2019
 *      Author: mbortolon
 */


#include "gtest/gtest.h"
#include "src/server/dataManagerServer/session/Session.hpp"
#include "src/common/dataManager/protocol/InitMessage.pb.h"
#include "src/common/dataManager/frameContainer/Frame.hpp"
#include "src/server/dataManagerServer/session/Client.hpp"

#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"

#include "boost/filesystem.hpp"

#include <iostream>
#include <string>
#include <sstream>

#define ADDITIONAL_DATA "aaaaaaaaaa"

#define RESULT_IDENTIFIER "TEST_TEST"

#define PHONEID_1 "PHONE1"

#define PHONEID_2 "PHONE2"

dataManagerServer::session::Session* jpegDecoder;

dataManager::frameContainer::Frame* createFrame() {
	cv::Mat decodedImage = cv::Mat::zeros(640, 480, CV_8UC3);
	std::vector<uchar> encodedImage = std::vector<uchar>();
	cv::imencode(".jpg", decodedImage, encodedImage);
	decodedImage.release();

	std::string* additionalData = new std::string(ADDITIONAL_DATA);
	std::string* frameEncoded = new std::string(encodedImage.begin(), encodedImage.end());

	encodedImage.clear();

	// Image processing
	dataManager::frameContainer::Frame* frame = new dataManager::frameContainer::Frame(0, frameEncoded, additionalData);

	return frame;
}

TEST(Session, SessionElaborateAndSaveImage) {
	// Image encoding

	dataManagerServer::session::Session* session = new dataManagerServer::session::Session(0, RESULT_IDENTIFIER, 2);

	std::string init_data = "";

	dataManagerServer::session::Client* client1 = session->addClient(PHONEID_1, true, dataManager::protocol::InitMessage_ContentCodec::InitMessage_ContentCodec_MJPEG, init_data);
	dataManagerServer::session::Client* client2 = session->addClient(PHONEID_2, true, dataManager::protocol::InitMessage_ContentCodec::InitMessage_ContentCodec_MJPEG, init_data);

	client1->enqueueFrame(createFrame());
	client2->enqueueFrame(createFrame());

	session->closeSession(1);

	std::ostringstream resultFolderStream;
	resultFolderStream << "/tmp/uploadedImages/" << RESULT_IDENTIFIER << "/";
	std::string resultFolder = resultFolderStream.str();

	std::ostringstream phoneOneResultFolderStream;
	phoneOneResultFolderStream << resultFolder << PHONEID_1 << "/";
	std::string phoneOneResultFolder = phoneOneResultFolderStream.str();

	std::ostringstream phoneTwoResultFolderStream;
	phoneTwoResultFolderStream << resultFolder << PHONEID_2 << "/";
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

	delete session;
}
