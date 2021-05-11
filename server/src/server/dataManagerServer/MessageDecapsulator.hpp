/*
 * MessageDecapsulator.hpp
 *
 *  Created on: May 8, 2019
 *      Author: mbortolon
 */

#ifndef SRC_DATAMANAGERSERVER_MESSAGEDECAPSULATOR_HPP_
#define SRC_DATAMANAGERSERVER_MESSAGEDECAPSULATOR_HPP_

#define __SAVE_DBG_FILE 1

#include <cstddef>
#include <iostream>

#include "src/common/dataManager/protocol/MainMessage.pb.h"
#include "src/common/dataManager/protocol/DataMessage.pb.h"
#include "session/Client.hpp"
#include "session/Session.hpp"

namespace dataManagerServer {

class MessageDecapsulator {
public:
	MessageDecapsulator();
	virtual ~MessageDecapsulator();

	enum ResponseType { ERROR_CLOSE_SOCKET, OK_NO_ANSWER, OK_SEND_RESPONSE };
	struct Response {
		Response(ResponseType type, std::string* response);
		virtual ~Response();
		ResponseType type;
		std::string* response;
	};

	Response ElaborateMessage(const void* array, size_t dimension);

	const bool isHost() const { return _isHost; }

	const dataManagerServer::session::Session* session() const { return _session; }

	const dataManagerServer::session::Client* currentClient() const { return _currentClient; }

private:
	dataManager::protocol::MainMessage_Type status = dataManager::protocol::MainMessage_Type::MainMessage_Type_INIT;
	bool _isHost = false;

	dataManagerServer::session::Session* _session;
	dataManagerServer::session::Client* _currentClient;

	dataManager::protocol::MainMessage mainMessage;
	dataManager::protocol::DataMessage dataMessage;
	dataManager::protocol::DataMessageResponse dataMessageResponse;

	bool ValidateInitMessage(dataManager::protocol::InitMessage& initMessage);
	bool InitSessionAndClient(dataManager::protocol::InitMessage& initMessage);

#ifdef __SAVE_DBG_FILE
	std::ofstream decapsulateFrameLoggerFile;
#endif
	std::string* InitSessionResponse();

	// Specific message types parsing systems
	Response ElaborateInitMessage();
	Response ElaborateDataMessage();
	Response ElaborateEndMessage();

};

} /* namespace dataManagerServer */

#endif /* SRC_DATAMANAGERSERVER_SESSION_CLIENT_HPP_ */
