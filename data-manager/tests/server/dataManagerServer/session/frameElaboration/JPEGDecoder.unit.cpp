#include "gtest/gtest.h"
#include "src/server/dataManagerServer/session/frameElaboration/JPEGDecoder.hpp"
#include "src/common/dataManager/frameContainer/Frame.hpp"
#include "src/common/dataManager/frameContainer/FrameDecapsulate.hpp"

#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"

#include <iostream>
#include <string>

#define ADDITIONAL_DATA "aaaaaaaaaa"

dataManagerServer::session::frameElaboration::JPEGDecoder* jpegDecoder;

dataManager::frameContainer::FrameDecapsulate* simulateTransmission() {
	std::string initData = "";
	jpegDecoder = new dataManagerServer::session::frameElaboration::JPEGDecoder();
	jpegDecoder->init(initData);

	// Image encoding
	cv::Mat decodedImage = cv::Mat::zeros(640, 480, CV_8UC3);
	std::vector<uchar> encodedImage = std::vector<uchar>();
	cv::imencode(".jpg", decodedImage, encodedImage);

	std::string* additionalData = new std::string(ADDITIONAL_DATA);
	std::string* frameEncoded = new std::string(encodedImage.begin(), encodedImage.end());

	encodedImage.clear();

	// Image processing
	dataManager::frameContainer::Frame* frame = new dataManager::frameContainer::Frame(0, frameEncoded, additionalData);
	dataManager::frameContainer::FrameDecapsulate* decapsulate = jpegDecoder->elaborateFrame(frame);

	delete jpegDecoder;

	return decapsulate;
}

TEST(JPEGDecoder, ImageNotEmptyAndValid) {
	dataManager::frameContainer::FrameDecapsulate* decapsulate = simulateTransmission();

	// Check function return no error
	ASSERT_NE(decapsulate, nullptr);

	// Check image size not null
	ASSERT_NE(decapsulate->frame->size(), 0);


	// Decode image
	cv::Mat rawData = cv::Mat(*(decapsulate->frame));
	cv::Mat resultDecodedImage = cv::imdecode(rawData, cv::ImreadModes::IMREAD_COLOR);

	// Check image decode correctly
	EXPECT_NE(resultDecodedImage.data, nullptr);

	delete decapsulate;
}

TEST(JPEGDecoder, SameAdditionalDataPointer) {
	dataManager::frameContainer::FrameDecapsulate* decapsulate = simulateTransmission();

	// Check array remain in the same position
	EXPECT_EQ(decapsulate->additionalData->size(), std::string(ADDITIONAL_DATA).size());

	delete decapsulate;
}
