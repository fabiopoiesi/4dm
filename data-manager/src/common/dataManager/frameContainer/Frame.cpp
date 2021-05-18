/*
 * Frame.cpp
 *
 *  Created on: Apr 23, 2019
 *      Author: mbortolon
 */

#include "Frame.hpp"

namespace dataManager {
namespace frameContainer {

Frame::Frame(uint64_t frameId, std::string* frame, std::string* additionalData) {
	this->frameId = frameId;
	this->frame = frame;
	this->additionalData = additionalData;
}

Frame::~Frame() {
	this->clear(false);
}

void Frame::clear(bool dontDeleteAdditionalData, bool dontDeleteFrame) {
	if(this->additionalData != nullptr) {
		if(dontDeleteAdditionalData == false) {
			delete this->additionalData;
		}
		this->additionalData = nullptr;
	}
	if(this->frame != nullptr) {
		if(dontDeleteFrame == false) {
			delete this->frame;
		}
		this->frame = nullptr;
	}
}

} /* namespace frameContainer */
} /* namespace dataManager */
