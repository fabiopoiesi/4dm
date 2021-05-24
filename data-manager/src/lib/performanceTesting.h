/*
 * performanceTesting.h
 *
 *  Created on: May 15, 2019
 *      Author: mbortolon
 */

#ifndef SRC_LIB_PERFORMANCETESTING_H_
#define SRC_LIB_PERFORMANCETESTING_H_

#include <stdint.h>

namespace Performance {
	extern "C" bool getStatus();
	extern "C" char* getErrorMessage();
	extern "C" uint64_t getAvgRate();
	extern "C" uint64_t getTotalByteSend();
	extern "C" uint32_t getAverageWriteTimeInNs();
	extern "C" uint64_t getPacketsSend();

	extern "C" void performanceTesting(const char* serverIpAddress, uint32_t serverPort);
}

#endif /* SRC_LIB_PERFORMANCETESTING_H_ */
