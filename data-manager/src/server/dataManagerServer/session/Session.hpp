/*
 * Session.hpp
 *
 *  Created on: Apr 23, 2019
 *      Author: mbortolon
 */

#ifndef SRC_DATAMANAGERSERVER_SESSION_SESSION_HPP_
#define SRC_DATAMANAGERSERVER_SESSION_SESSION_HPP_

#include <string>
#include <unordered_map>
#include <vector>
#include "concurrentqueue.h"

#include "Client.hpp"
#include "src/common/dataManager/protocol/InitMessage.pb.h"
#include "boost/thread.hpp"
#include "boost/filesystem.hpp"

namespace dataManagerServer {
namespace session {

class Session {
public:
	Session(int internalIdentifier, std::string resultIdentifier, unsigned int numberOfClients);
	virtual ~Session();

	Client* addClient(std::string phoneId, bool isHost, dataManager::protocol::InitMessage_ContentCodec framesCodec, std::string& initTransmissionData);
	Client* rejoinClient(std::string phoneId);

	const int internalIdentifier() const { return _internalIdentifier; }
	const std::string& resultIdentifier() const { return _resultIdentifier; }
	const int numberOfClients() const { return _numberOfClients; }

	void closeSession(uint64_t totalFrameSend);

	void frameElaborate(uint64_t frameID);
private:
	int _internalIdentifier;
	std::string _resultIdentifier;
	int _numberOfClients;

	boost::filesystem::path savePath;

	int _connectedClient = 0;

	Client** clients;

    moodycamel::ConcurrentQueue<uint64_t> framesReceiveQueue;

	std::unordered_map<uint64_t, unsigned short> framesReceive;

	bool closeSessionSignal = false;

	// Thread references
	void elaborateFrameFunction();
	void elaborationFrameQueuesFlushing();

	void FrameMergingAndSaveFunction();
	void frameMergingAndSaveQueueFlushing();

	void startFramesDecapsulationThread();

	void startFrameMergingThread();

	void stopThread();

	boost::thread* threadFramesElaboration;
	boost::thread* threadFrameMergingAndSave;

	void partialImageElaboration();
};

} /* namespace session */
} /* namespace dataManagerServer */

#endif /* SRC_DATAMANAGERSERVER_SESSION_SESSION_HPP_ */
