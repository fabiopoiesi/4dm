/*
 * imageSending.hpp
 *
 *  Created on: May 15, 2019
 *      Author: mbortolon
 */

#ifndef SRC_LIB_IMAGESENDING_H_
#define SRC_LIB_IMAGESENDING_H_

#include <stdint.h>
#include <cstddef>

namespace ImageSending {
	extern "C" unsigned int initSendingImage(uint64_t maximumRate, const char* libDebugFolder);

	extern "C" void setSessionClient(unsigned int position, uint32_t clients);

	extern "C" void setSessionIdentifier(unsigned int position, const char* streamId);

	extern "C" bool createConnection(unsigned int position, const char* serverIpAddress, uint32_t serverPort, const char* phoneId);

	extern "C" char* getStreamId(unsigned int position);

	extern "C" void sendingImageThreadFuction(unsigned int position);

	extern "C" void setMaximumRate(unsigned int position, uint64_t maximumRate);

	extern "C" bool sendImage(unsigned int position, uint64_t frameId, const unsigned char* y_buffer, const unsigned char* uv_buffer, int width, int height, const char* additionalData, size_t additionalDataLenght);
		
	extern "C" void sendingComplete(unsigned int position, int64_t totalFrame);

	extern "C" bool isSendComplete(unsigned int position);
}

#endif /* SRC_LIB_IMAGESENDING_H_ */
