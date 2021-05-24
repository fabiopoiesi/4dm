/*
 * JPEGDecoder.cpp
 *
 *  Created on: Apr 23, 2019
 *      Author: mbortolon
 */

#include "JPEGDecoder.hpp"

#include <iostream>
#include <vector>
#include <cstddef>

#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"

// #define __DBG_INFO 1

namespace dataManagerServer {
namespace session {
namespace frameElaboration {

JPEGDecoder::~JPEGDecoder() {
	this->params.clear();
}

bool JPEGDecoder::init(std::string& initBytes) {
	this->params.push_back(cv::IMWRITE_JPEG_QUALITY);
	this->params.push_back(98);
	return true;
}

dataManager::frameContainer::FrameDecapsulate* JPEGDecoder::elaborateFrame(
		dataManager::frameContainer::Frame* frame) {
#ifdef __DBG_INFO
	const char* dataPointer = frame->frame->data();

	std::cout << "Receive data array (JPEGDeca): [";
	for(unsigned int c = 0; c < 10; c++) {
		std::cout << (static_cast<int>(dataPointer[c]) & 0xFF) << " ";
	}
	std::cout << "... ";
	for(unsigned int c = frame->frame->size() - 10; c < frame->frame->size(); c++) {
		std::cout << (static_cast<int>(dataPointer[c]) & 0xFF) << " ";
	}
	std::cout << "]" << std::endl;
#endif

	std::vector<unsigned char> vectordata(frame->frame->begin(),frame->frame->end());
	cv::Mat rawData(vectordata,true);
	//cv::Mat rawData(1, frame->frame->size(), CV_8UC1, (void*)frame->frame->data());
	cv::Mat decodedImage = cv::imdecode(rawData, cv::ImreadModes::IMREAD_COLOR);
	if (decodedImage.data == NULL) {
		std::cout << "Image not encode correctly" << std::endl;
		return nullptr;
	}
#ifdef __DBG_INFO
	std::cout << "decodedImage = (" << decodedImage.rows << ", " << decodedImage.cols << ")" << std::endl;
#endif
	if (decodedImage.data == NULL) {
		std::cout << "Image not encode correctly" << std::endl;
		return nullptr;
	}
	std::vector<uchar>* encodedImage = new std::vector<uchar>();
	cv::imencode(".jpg", decodedImage, *encodedImage, this->params);

#ifdef __DBG_INFO
	const unsigned char* dataPointerEncode = encodedImage->data();

	std::cout << "Receive data array (JPEG re-encoded): [";
	for(unsigned int c = 0; c < 10; c++) {
		std::cout << (static_cast<int>(dataPointerEncode[c]) & 0xFF) << " ";
	}
	std::cout << "... ";
	for(unsigned int c = encodedImage->size() - 10; c < encodedImage->size(); c++) {
		std::cout << (static_cast<int>(dataPointerEncode[c]) & 0xFF) << " ";
	}
	std::cout << "]" << std::endl;
#endif
	dataManager::frameContainer::FrameDecapsulate* outputFrame =
			new dataManager::frameContainer::FrameDecapsulate(frame->frameId, encodedImage,
					frame->additionalData);

	frame->clear(true, false);
	delete frame;

	return outputFrame;
}

} /* namespace frameElaboration */
} /* namespace session */
} /* namespace dataManagerServer */
