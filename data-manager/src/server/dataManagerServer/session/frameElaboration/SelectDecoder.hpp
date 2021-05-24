/*
 * AbstractDecoder.hpp
 *
 *  Created on: Apr 23, 2019
 *      Author: mbortolon
 */

#ifndef SRC_DATAMANAGERSERVER_SESSION_FRAMEELABORATION_SELECTDECODER_HPP_
#define SRC_DATAMANAGERSERVER_SESSION_FRAMEELABORATION_SELECTDECODER_HPP_

#include "AbstractDecoder.hpp"

#include "src/common/dataManager/protocol/InitMessage.pb.h"

namespace dataManagerServer {
namespace session {
namespace frameElaboration {

dataManagerServer::session::frameElaboration::AbstractDecoder* SelectCorrectDecoder(dataManager::protocol::InitMessage_ContentCodec codec);

} /* namespace frameElaboration */
} /* namespace session */
} /* namespace dataManagerServer */

#endif /* SRC_DATAMANAGERSERVER_SESSION_FRAMEELABORATION_SELECT_DECODER_HPP_ */
