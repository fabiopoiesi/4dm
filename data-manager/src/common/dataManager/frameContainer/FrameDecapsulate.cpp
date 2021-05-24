/*
 * FrameDecapsulate.cpp
 *
 *  Created on: Apr 24, 2019
 *      Author: mbortolon
 */

#include "FrameDecapsulate.hpp"

namespace dataManager {
namespace frameContainer {

FrameDecapsulate::FrameDecapsulate(uint64_t frameId, std::vector<unsigned char>* frame, std::string* additionalData) {
	this->frameId = frameId;
	this->frame = frame;
	this->additionalData = additionalData;
}

FrameDecapsulate::~FrameDecapsulate() {
	if(this->additionalData != nullptr) {
		delete this->additionalData;
		this->additionalData = nullptr;
	}
	if(this->frame != nullptr) {
		delete this->frame;
		this->frame = nullptr;
	}
}

} /* namespace frameContainer */
} /* namespace dataManager */
