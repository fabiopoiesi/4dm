/*
 * Session.cpp
 *
 *  Created on: Apr 23, 2019
 *      Author: mbortolon
 */

#include "Session.hpp"
#include <iostream>
#include "boost/bind.hpp"

namespace dataManagerServer {
namespace session {

Session::Session(int internalIdentifier, std::string resultIdentifier, unsigned int numberOfClients) : framesReceiveQueue(150) {
	this->_internalIdentifier = internalIdentifier;
	this->_resultIdentifier = resultIdentifier;
	this->_numberOfClients = numberOfClients;
	this->clients = new Client*[numberOfClients];
	for(int i = 0; i < this->_numberOfClients; i++) {
		this->clients[i] = nullptr;
	}

	// Create directory structure where save the file
	std::stringstream ss;
	ss << "/tmp/uploadedImages/" << this->_resultIdentifier;
	this->savePath = ss.str();
	boost::filesystem::create_directories(this->savePath);

	this->threadFramesElaboration = nullptr;
	this->threadFrameMergingAndSave = nullptr;

	this->startFramesDecapsulationThread();
	this->startFrameMergingThread();
}

Session::~Session() {
	this->stopThread();
	for(int i = 0; i < this->_numberOfClients; i++) {
		if(this->clients[i] != nullptr) {
			delete this->clients[i];
		}
	}
	delete[] this->clients;
	this->clients = nullptr;
}

Client* Session::addClient(std::string phoneId, bool isHost, dataManager::protocol::InitMessage_ContentCodec framesCodec, std::string& initTransmissionData) {
	if(this->_connectedClient >= this->_numberOfClients) {
		std::cout << "Maximum number of client available on this session reach" << std::endl;
		return nullptr;
	}
	Client* client = new Client(this->_connectedClient, phoneId, isHost, framesCodec, initTransmissionData, this->savePath);
	this->clients[this->_connectedClient] = client;
	this->_connectedClient++;
	return client;
}

Client* Session::rejoinClient(std::string phoneId) {
	for(int i = 0; i < this->_connectedClient; i++) {
		if(this->clients[i]->phoneId() == phoneId) {
			return this->clients[i];
		}
	}
	return nullptr;
}

void Session::closeSession(uint64_t totalFrameSend) {
	this->stopThread();

	// TODO: implement statistic calculation
}

void Session::frameElaborate(uint64_t frameID) {
	this->framesReceiveQueue.enqueue(frameID);
}

void Session::stopThread() {
	this->closeSessionSignal = true;

	if (this->threadFramesElaboration != nullptr) {
		this->threadFramesElaboration->join();
	}

	if (this->threadFrameMergingAndSave != nullptr) {
		this->threadFrameMergingAndSave->join();
	}

	this->elaborationFrameQueuesFlushing();
	this->frameMergingAndSaveQueueFlushing();

	this->partialImageElaboration();

	if (this->threadFramesElaboration != nullptr) {
		delete this->threadFramesElaboration;
		this->threadFramesElaboration = nullptr;
	}
	if (this->threadFrameMergingAndSave != nullptr) {
		delete this->threadFrameMergingAndSave;
		this->threadFrameMergingAndSave = nullptr;
	}
}

void Session::partialImageElaboration() {
	for (auto& element: this->framesReceive) {
		std::ostringstream filesName;
		filesName << std::setfill('0') << std::setw(5) << element.first << "-partial";
		std::string filesNameString = filesName.str();

		for(int i = 0; i < this->_numberOfClients; i++) {
			if(this->clients[i] != nullptr) {
				this->clients[i]->saveFrameIfExist(element.first,filesNameString);
	                }
		}

		std::cout << "[Merging] Partial Frame " << element.first << " saved" << std::endl;
	}
	this->framesReceive.clear();
}

void Session::startFramesDecapsulationThread() {
	this->threadFramesElaboration = new boost::thread(boost::bind(&Session::elaborateFrameFunction, this));
}

void Session::startFrameMergingThread() {
	this->threadFrameMergingAndSave = new boost::thread(boost::bind(&Session::FrameMergingAndSaveFunction, this));
}

void Session::elaborationFrameQueuesFlushing() {
	// This cycle continue until all frame of all clients was elaborated
	bool otherFrame = false;
	do {
		otherFrame = false;
		// Decapsulate one frame for each client if available
		for(int i = 0; i < this->_numberOfClients; i++) {
			if(this->clients[i] != nullptr) {
				long result = this->clients[i]->decapsulateOneFrameIfAvailable();
				if(result == -1) {
					continue;
				}
				otherFrame = true;
				this->framesReceiveQueue.enqueue(result);
				std::cout << "[" << this->clients[i]->phoneId() << "] Elaborate frame " << result << std::endl;
			}
		}
	} while(otherFrame);
}

void Session::elaborateFrameFunction() {
	// Continue until the session is close
	do {
		this->elaborationFrameQueuesFlushing();
		boost::this_thread::sleep(boost::posix_time::milliseconds(10));
	} while(!this->closeSessionSignal);
	std::cout << "Closing elaboration thread" << std::endl;
}

void Session::frameMergingAndSaveQueueFlushing() {
	// Continue until the queue is empty
	uint64_t frame = 0;
	while(framesReceiveQueue.try_dequeue(frame)) {
		std::unordered_map<uint64_t, unsigned short>::iterator search = this->framesReceive.find(frame);

		if(search == this->framesReceive.end()) {
			this->framesReceive[frame] = 1;
		} else {
			search->second += 1;
		}

		if(this->framesReceive[frame] >= this->_numberOfClients) {
			std::ostringstream filesName;
			filesName << std::setfill('0') << std::setw(5) << frame;
			std::string filesNameString = filesName.str();

			for(int i = 0; i < this->_numberOfClients; i++) {
				this->clients[i]->saveFrame(frame,filesNameString);
			}
			std::cout << "[Merging] Frame " << frame << " saved" << std::endl;

			if(this->_numberOfClients <= 1) {
				this->framesReceive.erase(frame);
			} else {
				this->framesReceive.erase(search);
			}
		}
	}
}

void Session::FrameMergingAndSaveFunction() {
	// Continue until the session is closes
	do {
		this->frameMergingAndSaveQueueFlushing();
		boost::this_thread::sleep(boost::posix_time::milliseconds(10));
	} while(!this->closeSessionSignal);
	std::cout << "Closing merging thread" << std::endl;
}

} /* namespace session */
} /* namespace dataManagerServer */
