/*
 * FrameDecapsulate.hpp
 *
 *  Created on: Apr 24, 2019
 *      Author: mbortolon
 */

#ifndef SRC_DATAMANAGERSERVER_SESSION_FRAMEDECAPSULATE_HPP_
#define SRC_DATAMANAGERSERVER_SESSION_FRAMEDECAPSULATE_HPP_

#include <vector>
#include <cstdint>
#include <string>

#include "opencv2/core/core.hpp"

namespace dataManager {
namespace frameContainer {

class FrameDecapsulate {
public:
	FrameDecapsulate(uint64_t frameId, std::vector<unsigned char>* frame, std::string* additionalData);
	virtual ~FrameDecapsulate();

	uint64_t frameId;
	std::vector<unsigned char>* frame;
	std::string* additionalData;
};

} /* namespace frameContainer */
} /* namespace dataManager */

#endif /* SRC_DATAMANAGERSERVER_SESSION_FRAMEDECAPSULATE_HPP_ */
