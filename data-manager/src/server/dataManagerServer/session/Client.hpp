/*
 * Client.hpp
 *
 *  Created on: Apr 23, 2019
 *      Author: mbortolon
 */

#ifndef SRC_DATAMANAGERSERVER_SESSION_CLIENT_HPP_
#define SRC_DATAMANAGERSERVER_SESSION_CLIENT_HPP_

#include <string>
#include <unordered_map>

#include "src/common/dataManager/protocol/InitMessage.pb.h"
#include "src/common/dataManager/frameContainer/Frame.hpp"
#include "src/common/dataManager/frameContainer/FrameDecapsulate.hpp"
#include "frameElaboration/AbstractDecoder.hpp"
#include "frameElaboration/SelectDecoder.hpp"

#include "concurrentqueue.h"

#include "boost/filesystem.hpp"

namespace dataManagerServer {
namespace session {

class Client {
public:
	Client(unsigned int internalIdentifier, std::string phoneId, bool isHost, dataManager::protocol::InitMessage_ContentCodec framesCodec, std::string& initTransmissionData, boost::filesystem::path& sessionSavePath);
	virtual ~Client();

	void enqueueFrame(dataManager::frameContainer::Frame* frame);

	long decapsulateOneFrameIfAvailable();

	bool saveFrame(uint64_t frameId, std::string& filesName);
        bool saveFrameIfExist(uint64_t frameId, std::string& filesName);

	const int internalIdentifier() const { return _internalIdentifier; }
	const std::string& phoneId() const { return _phoneId; }
	const bool isHost() const { return _isHost; }
	const boost::filesystem::path& savePath() const { return _savePath; }
private:
	int _internalIdentifier;
	std::string _phoneId;
	bool _isHost = false;

	boost::filesystem::path _savePath;

	frameElaboration::AbstractDecoder* decoder;

	dataManager::protocol::InitMessage_ContentCodec _framesCodec;

        moodycamel::ConcurrentQueue<dataManager::frameContainer::Frame*> framesToDecapsulate;

	std::unordered_map<uint64_t, dataManager::frameContainer::FrameDecapsulate*> frames;
};

} /* namespace session */
} /* namespace dataManagerServer */

#endif /* SRC_DATAMANAGERSERVER_SESSION_CLIENT_HPP_ */
