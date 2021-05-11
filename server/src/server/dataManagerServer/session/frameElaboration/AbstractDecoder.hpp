/*
 * AbstractDecoder.hpp
 *
 *  Created on: Apr 23, 2019
 *      Author: mbortolon
 */

#ifndef SRC_DATAMANAGERSERVER_SESSION_FRAMEELABORATION_ABSTRACTDECODER_HPP_
#define SRC_DATAMANAGERSERVER_SESSION_FRAMEELABORATION_ABSTRACTDECODER_HPP_

#include "opencv2/opencv.hpp"

#include "src/common/dataManager/frameContainer/FrameDecapsulate.hpp"
#include "src/common/dataManager/frameContainer/Frame.hpp"

#include <string>

namespace dataManagerServer {
namespace session {
namespace frameElaboration {

class AbstractDecoder {
public:
	AbstractDecoder();
	virtual ~AbstractDecoder();

	virtual bool init(std::string& initBytes) = 0;

	virtual dataManager::frameContainer::FrameDecapsulate* elaborateFrame(dataManager::frameContainer::Frame* frame) = 0;
};

} /* namespace frameElaboration */
} /* namespace session */
} /* namespace dataManagerServer */

#endif /* SRC_DATAMANAGERSERVER_SESSION_FRAMEELABORATION_ABSTRACTDECODER_HPP_ */
