/*
 * AbstractDecoder.cpp
 *
 *  Created on: Apr 23, 2019
 *      Author: mbortolon
 */
#include "SelectDecoder.hpp"

#include "JPEGDecoder.hpp"
#include <iostream>

namespace dataManagerServer {
namespace session {
namespace frameElaboration {

dataManagerServer::session::frameElaboration::AbstractDecoder* SelectCorrectDecoder(dataManager::protocol::InitMessage_ContentCodec codec) {
	switch(codec) {
	case dataManager::protocol::InitMessage_ContentCodec::InitMessage_ContentCodec_MJPEG:
		return new JPEGDecoder();
		break;
	default:
		std::cout << "Decoder not yet supported" << std::endl;
		return nullptr;
		break;
	}
}

} /* namespace frameElaboration */
} /* namespace session */
} /* namespace dataManagerServer */
