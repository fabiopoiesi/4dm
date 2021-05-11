/*
 * Decapsulator.hpp
 *
 *  Created on: May 8, 2019
 *      Author: mbortolon
 */

#ifndef SRC_COMMON_DATAMANAGERSERVER_ENCAPSULATION_DECAPSULATOR_HPP_
#define SRC_COMMON_DATAMANAGERSERVER_ENCAPSULATION_DECAPSULATOR_HPP_

#include <cstddef>
#include <functional>

#include "protocolLimit.hpp"

namespace dataManagerServer {
namespace encapsulation {

class Decapsulator {
public:
	Decapsulator(std::function<void(const void*, size_t)> onMessageCallback, std::function<void()> onTooLargePacket);
	virtual ~Decapsulator();

	void readBuffer(const unsigned char* buffer, size_t bufferSize);
private:
	std::function<void()> onTooLargePacket;
	std::function<void(const void*, size_t)> onMessageCallback;

        char protocolMessage[MESSAGE_MAX_SIZE];
        size_t messageToReceive = 0;
        unsigned int messageActualPosition = 0;
        bool messageDiscarded = false;
        size_t messageSize = 0;
};

} // namespace encapsulation
} // namespace dataManagerServer

#endif