/*
 * Frame.hpp
 *
 *  Created on: Apr 23, 2019
 *      Author: mbortolon
 */

#ifndef SRC_DATAMANAGERSERVER_SESSION_FRAME_HPP_
#define SRC_DATAMANAGERSERVER_SESSION_FRAME_HPP_

#include <cstdint>
#include <string>

namespace dataManager {
namespace frameContainer {

class Frame {
public:
	Frame(uint64_t frameId, std::string* frame, std::string* additionalData);
	virtual ~Frame();

	void clear(bool dontDeleteData = false, bool dontDeleteFrame = false);

	int64_t frameId;
	std::string* frame;
	std::string* additionalData;
};

} /* namespace frameContainer */
} /* namespace dataManager */

#endif /* SRC_DATAMANAGERSERVER_SESSION_FRAME_HPP_ */
