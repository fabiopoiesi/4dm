/*
 * Encapsulator.hpp
 *
 *  Created on: May 8, 2019
 *      Author: mbortolon
 */

#ifndef SRC_COMMON_DATAMANAGERSERVER_ENCAPSULATION_ENCAPSULATOR_HPP_
#define SRC_COMMON_DATAMANAGERSERVER_ENCAPSULATION_ENCAPSULATOR_HPP_

#include <cstddef>
#include <functional>

#include "protocolLimit.hpp"

namespace dataManagerServer {
namespace encapsulation {

class Encapsulator {
public:
	Encapsulator(std::function<ssize_t(const void*, size_t)> sendingFunction, uint64_t maximumRate);
	virtual ~Encapsulator();

	bool sendMessage(unsigned char* buffer, size_t size);

	void setMaximumRate(uint64_t maximumRate);
private:
	std::function<ssize_t(const void*, size_t)> sendingFunction;
	std::function<void(const void*, size_t)> onMessageCallback;

	void wait();

	char firstPacketBuffer[PACKET_MAX_SIZE] = { 0 };

	// Rate limiting functions
	uint64_t maximumRate;
	unsigned int sleepClock = 0;
};

} // namespace encapsulation
} // namespace dataManagerServer

#endif
