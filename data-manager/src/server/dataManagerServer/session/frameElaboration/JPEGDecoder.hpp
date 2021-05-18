/*
 * JPEGDecoder.hpp
 *
 *  Created on: Apr 23, 2019
 *      Author: mbortolon
 */

#ifndef SRC_DATAMANAGERSERVER_SESSION_FRAMEELABORATION_JPEGDECODER_HPP_
#define SRC_DATAMANAGERSERVER_SESSION_FRAMEELABORATION_JPEGDECODER_HPP_

#include "AbstractDecoder.hpp"

#include <string>
#include <vector>
#include "src/common/dataManager/frameContainer/FrameDecapsulate.hpp"
#include "src/common/dataManager/frameContainer/Frame.hpp"

namespace dataManagerServer {
namespace session {
namespace frameElaboration {

class JPEGDecoder: public dataManagerServer::session::frameElaboration::AbstractDecoder {
public:
	virtual ~JPEGDecoder();

	bool init(std::string& initBytes);

	dataManager::frameContainer::FrameDecapsulate* elaborateFrame(dataManager::frameContainer::Frame* frame);
private:
	std::vector<int> params;
};

} /* namespace frameElaboration */
} /* namespace session */
} /* namespace dataManagerServer */

#endif /* SRC_DATAMANAGERSERVER_SESSION_FRAMEELABORATION_JPEGDECODER_HPP_ */
