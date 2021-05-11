/*
 * MessageDecapsulator.cpp
 *
 *  Created on: May 8, 2019
 *      Author: mbortolon
 */

// #define __DBG_INFO 1
#define __SAVE_DBG_FILE 1

#include "MessageDecapsulator.hpp"

#include "src/common/dataManager/protocol/EndMessage.pb.h"
#include "src/common/dataManager/protocol/InitMessage.pb.h"
#include "session/SessionsManager.hpp"
#include "src/common/dataManager/frameContainer/Frame.hpp"
#include "src/common/dataManager/md5.h"

#include <streambuf>
#include <iostream>
#include <istream>
#include <ostream>
#include <cstring>
#include "boost/filesystem.hpp"

namespace dataManagerServer {

MessageDecapsulator::MessageDecapsulator() : mainMessage(), dataMessage() {
	this->_currentClient = nullptr;
	this->_isHost = false;
	this->_session = nullptr;
}

MessageDecapsulator::~MessageDecapsulator() {
#ifdef __SAVE_DBG_FILE
	if(this->decapsulateFrameLoggerFile.is_open()) {
		this->decapsulateFrameLoggerFile.close();
	}
#endif
}

MessageDecapsulator::Response::Response(ResponseType type, std::string* response) {
	this->type = type;
	this->response = response;
}

MessageDecapsulator::Response::~Response() {
	if(this->response != nullptr) {
		delete this->response;
	}
}

struct OneShotReadBuf : public std::streambuf
{
    OneShotReadBuf(char* s, std::size_t n)
    {
        setg(s, s, s + n);
    }
};

MessageDecapsulator::Response MessageDecapsulator::ElaborateMessage(const void* array, size_t dimension) {
	// Main message parsing
	bool mainMessageParsing = this->mainMessage.ParseFromArray(array, static_cast<int>(dimension));
	if(mainMessageParsing == false) {
		std::cout << "Main message can't be parsed" << std::endl;
		this->mainMessage.Clear();
		return MessageDecapsulator::Response(MessageDecapsulator::ResponseType::ERROR_CLOSE_SOCKET, nullptr);
	}

	if(this->status == dataManager::protocol::MainMessage_Type::MainMessage_Type_INIT) {
		return this->ElaborateInitMessage();
	} else if(this->mainMessage.type() == dataManager::protocol::MainMessage_Type::MainMessage_Type_DATA) {
		return this->ElaborateDataMessage();
	} else if(this->mainMessage.type() == dataManager::protocol::MainMessage_Type::MainMessage_Type_END) {
		return this->ElaborateEndMessage();
	} else {
		std::cout << "Unknown message type received" << std::endl;
	}

	return MessageDecapsulator::Response(MessageDecapsulator::ResponseType::ERROR_CLOSE_SOCKET, nullptr);
}

std::string* MessageDecapsulator::InitSessionResponse() {
	dataManager::protocol::InitMessageResponse response = dataManager::protocol::InitMessageResponse();
	response.set_streamid(this->_session->resultIdentifier());
	std::string encoded = response.SerializeAsString();
	response.Clear();
	return new std::string(encoded);
}

bool MessageDecapsulator::ValidateInitMessage(dataManager::protocol::InitMessage& initMessage) {
	if(initMessage.applicationid().length() == 0) {
		std::cout << "Application not authenticated" << std::endl;
		// TODO: test application authentication
		return false;
	}

	if(initMessage.phoneid().length() == 0) {
		std::cout << "PhoneId can't be empty" << std::endl;
		return false;
	}

	if(initMessage.is_host_or_client_case() == dataManager::protocol::InitMessage::IsHostOrClientCase::IS_HOST_OR_CLIENT_NOT_SET) {
		std::cout << "clients or streamID need to be set" << std::endl;
		return false;
	}

	return true;
}

bool MessageDecapsulator::InitSessionAndClient(dataManager::protocol::InitMessage& initMessage) {
	dataManagerServer::session::SessionsManager* instance = dataManagerServer::session::SessionsManager::getInstance();

	if(initMessage.rejoin() == true) {
		std::cout << "Client rejoin" << std::endl;
		this->_isHost = true;
		this->_session = instance->findSession(initMessage.streamid());
		if(this->_session == nullptr) {
			std::cout << "Given session don't exist, given streamId: " << initMessage.streamid() << std::endl;
			return false;
		}
		this->_currentClient = this->_session->rejoinClient(initMessage.phoneid());
		if(this->_currentClient == nullptr) {
			std::cout << "Rejoin client not found" << std::endl;
			return false;
		}
		return true;
	}

	if(initMessage.is_host_or_client_case() == dataManager::protocol::InitMessage::IsHostOrClientCase::kClients) {
		this->_isHost = true;
		this->_session = instance->createNewSession(initMessage.clients());
	} else {
		this->_isHost = false;
		this->_session = instance->findSession(initMessage.streamid());
		if(this->_session == nullptr) {
			std::cout << "Given session don't exist, given streamId: " << initMessage.streamid() << std::endl;
			return false;
		}
	}

	std::string phoneId = initMessage.phoneid();

#ifdef __SAVE_DBG_FILE
	boost::filesystem::path savePath = boost::filesystem::path("/tmp/uploadedImages") / boost::filesystem::path(this->_session->resultIdentifier()) / boost::filesystem::path(std::string("receiveFrames") + phoneId + std::string(".txt"));
	this->decapsulateFrameLoggerFile.open(savePath.c_str(), std::ios::out | std::ios::app);
#endif

	std::string initBytes = initMessage.initbytes();
	this->_currentClient = this->_session->addClient(phoneId, this->_isHost, initMessage.contentcodec(), initBytes);

	return true;
}

// ++++++++++++++++++++++++++++++++++++++++++
// +                                        +
// + Specific message types parsing systems +
// +                                        +
// ++++++++++++++++++++++++++++++++++++++++++

MessageDecapsulator::Response MessageDecapsulator::ElaborateInitMessage() {
	// Init message
	if(!(this->mainMessage.type() == dataManager::protocol::MainMessage_Type::MainMessage_Type_INIT)) {
		std::cout << "Expected an initialization message, but another type of message received" << std::endl;
		this->mainMessage.Clear();
		return MessageDecapsulator::Response(MessageDecapsulator::ResponseType::ERROR_CLOSE_SOCKET, nullptr);
	}
	dataManager::protocol::InitMessage initMessage = dataManager::protocol::InitMessage();
	bool contentParsingResult = initMessage.ParseFromString(this->mainMessage.info());
	if(contentParsingResult == false) {
		std::cout << "Init message can't be parsed" << std::endl;
		this->mainMessage.Clear();
		initMessage.Clear();
		return MessageDecapsulator::Response(MessageDecapsulator::ResponseType::ERROR_CLOSE_SOCKET, nullptr);
	}

	if(!this->ValidateInitMessage(initMessage)) {
		this->mainMessage.Clear();
		initMessage.Clear();
		return MessageDecapsulator::Response(MessageDecapsulator::ResponseType::ERROR_CLOSE_SOCKET, nullptr);
	}

	bool result = this->InitSessionAndClient(initMessage);
	if(!result) {
		return MessageDecapsulator::Response(MessageDecapsulator::ResponseType::ERROR_CLOSE_SOCKET, nullptr);
	}

	this->status = dataManager::protocol::MainMessage_Type::MainMessage_Type_DATA;

	if(this->_isHost && initMessage.rejoin() == false) {
		std::string* response = this->InitSessionResponse();
		initMessage.Clear();
		this->mainMessage.Clear();
		return MessageDecapsulator::Response(MessageDecapsulator::ResponseType::OK_SEND_RESPONSE, response);
	}

	initMessage.Clear();
	this->mainMessage.Clear();
	return MessageDecapsulator::Response(MessageDecapsulator::ResponseType::OK_NO_ANSWER, nullptr);
}

MessageDecapsulator::Response MessageDecapsulator::ElaborateDataMessage() {
	std::cout << "data message" << std::endl;
	// Data message
	bool contentParsingResult = this->dataMessage.ParseFromString(this->mainMessage.info());
	if(contentParsingResult == false) {
		std::cout << "Data message can't be parsed" << std::endl;
		this->mainMessage.Clear();
		return MessageDecapsulator::Response(MessageDecapsulator::ResponseType::ERROR_CLOSE_SOCKET, nullptr);
	}

	std::string* additionalData = new std::string(this->dataMessage.additionaldata());
	std::string* frameEncoded = new std::string(this->dataMessage.data());
	std::string* hashFrameEncoded = new std::string(this->dataMessage.datahash());

	MD5 md5;
	char* hash = md5.digestMemory((unsigned char*)(frameEncoded->c_str()), frameEncoded->size());
#ifdef __DBG_INFO
	std::cout << "Server calculate hash is: [";
	for(int i = 0; i < 16; i++) {
		std::cout << hash[i] << " ";
	}
	std::cout << "]" << std::endl;
#endif
	if(std::memcmp(hash, hashFrameEncoded->c_str(), 16) != 0) {
		//TODO: send a message that hash is wrong
		std::cout << "Message with wrong hash, retransmission required" << std::endl;
		this->dataMessageResponse.Clear();
		this->dataMessageResponse.set_status(0);

		this->mainMessage.Clear();
		this->dataMessage.Clear();
	}

#ifdef __DBG_INFO
	const char* dataPointer = frameEncoded->data();
	std::cout << "Receive data array (msgdeca): [";
	for(unsigned int c = 0; c < 10; c++) {
		std::cout << (static_cast<int>(dataPointer[c]) & 0xFF) << " ";
	}
	std::cout << "... ";
	for(unsigned int c = frameEncoded->size() - 10; c < frameEncoded->size(); c++) {
		std::cout << (static_cast<int>(dataPointer[c]) & 0xFF) << " ";
	}
	std::cout << "]" << std::endl;
#endif
#ifdef __SAVE_DBG_FILE
	this->decapsulateFrameLoggerFile << this->dataMessage.frameid() << std::endl;
#endif
	dataManager::frameContainer::Frame* frame = new dataManager::frameContainer::Frame(this->dataMessage.frameid(), frameEncoded, additionalData);
	this->_currentClient->enqueueFrame(frame);

	this->mainMessage.Clear();
	this->dataMessage.Clear();
	return MessageDecapsulator::Response(MessageDecapsulator::ResponseType::OK_NO_ANSWER, nullptr);
}

MessageDecapsulator::Response MessageDecapsulator::ElaborateEndMessage() {
	// End message
	dataManager::protocol::EndMessage endMessage = dataManager::protocol::EndMessage();
	bool contentParsingResult = endMessage.ParseFromString(this->mainMessage.info());
	if(contentParsingResult == false) {
		std::cout << "End message can't be parsed" << std::endl;
		this->mainMessage.Clear();
		return MessageDecapsulator::Response(MessageDecapsulator::ResponseType::ERROR_CLOSE_SOCKET, nullptr);
	}

	dataManagerServer::session::SessionsManager* instance = dataManagerServer::session::SessionsManager::getInstance();
	instance->deleteSession(this->_session);
	this->_session = nullptr;

	this->mainMessage.Clear();
	return MessageDecapsulator::Response(MessageDecapsulator::ResponseType::ERROR_CLOSE_SOCKET, nullptr);
}

} /* namespace dataManagerServer */
