/*
 * Client.cpp
 *
 *  Created on: Apr 23, 2019
 *      Author: mbortolon
 */

#include "Client.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>

namespace dataManagerServer {
namespace session {

Client::Client(unsigned int internalIdentifier, std::string phoneId, bool isHost, dataManager::protocol::InitMessage_ContentCodec framesCodec, std::string& initTransmissionData, boost::filesystem::path& sessionSavePath) : framesToDecapsulate(150) {
        this->_internalIdentifier = internalIdentifier;
	this->_phoneId = phoneId;
	this->_isHost = isHost;
	this->_framesCodec = framesCodec;
	this->decoder = frameElaboration::SelectCorrectDecoder(this->_framesCodec);
	this->decoder->init(initTransmissionData);
	this->_savePath = sessionSavePath / this->_phoneId;
	boost::filesystem::create_directory(this->_savePath);

	std::cout << "New client join" << std::endl;
}

void Client::enqueueFrame(dataManager::frameContainer::Frame* frame) {
        this->framesToDecapsulate.enqueue(frame);
}

long Client::decapsulateOneFrameIfAvailable() {
        dataManager::frameContainer::Frame* frame;
	bool result = this->framesToDecapsulate.try_dequeue(frame);
	if(result == false) {
		return -1;
	}
	dataManager::frameContainer::FrameDecapsulate* decapsulate = this->decoder->elaborateFrame(frame);
	this->frames[decapsulate->frameId] = decapsulate;

	return decapsulate->frameId;
}

Client::~Client() {
	delete this->decoder;
	this->decoder = nullptr;
	std::unordered_map<uint64_t, dataManager::frameContainer::FrameDecapsulate*>::iterator it = this->frames.begin();
	while(it != this->frames.end()) {
		delete it->second;
	}
	this->frames.clear();
        dataManager::frameContainer::Frame* frame;
	while(this->framesToDecapsulate.try_dequeue(frame)) {
		delete frame;
	}
	frame = nullptr;
}

bool Client::saveFrame(uint64_t frameId, std::string& filesName) {
	std::unordered_map<uint64_t, dataManager::frameContainer::FrameDecapsulate*>::iterator element = this->frames.find(frameId);
	if(element == this->frames.end()) {
		std::cout << "Expect element not found" << std::endl;
		return false;
	}

#ifdef __DBG_INFO
			unsigned char* dataPointer = item->frame->data();
			std::cout << "Send data array: [";
			for(unsigned int c = 0; c < 10; c++) {
				std::cout << (unsigned short)(dataPointer[c]) << " ";
			}
			std::cout << "... ";
			for(unsigned int c = item->frame->size() - 10; c < item->frame->size(); c++) {
				std::cout << (unsigned short)(dataPointer[c]) << " ";
			}
			std::cout << "]" << std::endl;
#endif

#ifdef __DBG_INFO
			std::ostringstream frameSavePath;
			frameSavePath << "/tmp/outputImage" << frameId << ".jpg";
			std::ofstream textout(frameSavePath.str(), std::ios::out | std::ios::binary);
			textout.write((const char*)&(*imageBuffer)[0], imageBuffer->size());
			textout.close();
#endif

	boost::filesystem::path imagePath = this->_savePath / (filesName + ".jpg");
	std::ofstream frameFile(imagePath.generic().c_str(), std::ios::out | std::ios::binary);
	frameFile.write((const char*)element->second->frame->data(), element->second->frame->size());
	frameFile.close();

	boost::filesystem::path additionalDataPath = this->_savePath / (filesName + ".bin");

	// TODO: Additional data elaboration library
	std::ofstream additionalDataFile(additionalDataPath.generic().c_str(), std::ios::out | std::ios::binary);
	additionalDataFile.write(element->second->additionalData->c_str(), element->second->additionalData->size());
	additionalDataFile.close();

	delete element->second;
	this->frames.erase(element);

	return true;
}

bool Client::saveFrameIfExist(uint64_t frameId, std::string& filesName) {
	if(this->frames.find(frameId) != this->frames.end())
	{
		return this->saveFrame(frameId, filesName);
	}
	return false;
}

} /* namespace session */
} /* namespace dataManagerServer */
