/*
 * SessionsManager.cpp
 *
 *  Created on: Apr 23, 2019
 *      Author: mbortolon
 */

#include "SessionsManager.hpp"

#include <random>

#define IDENTIFICATOR_LENGTH 10
#define MAX_INTERNAL_IDENTIFICATOR 5

namespace dataManagerServer {
namespace session {

SessionsManager* SessionsManager::instance = nullptr;

SessionsManager* SessionsManager::getInstance() {
	if(SessionsManager::instance == nullptr) {
		SessionsManager::instance = new SessionsManager();
	}
	return SessionsManager::instance;
}

SessionsManager::SessionsManager() {
	srand(time(NULL));
}

SessionsManager::~SessionsManager() {
	// TODO Auto-generated destructor stub
}

Session* SessionsManager::createNewSession(unsigned int numberOfClients) {
	unsigned int internalIdentifier = rand() % MAX_INTERNAL_IDENTIFICATOR;
	Session* session = new Session(
			internalIdentifier,
			this->sessionStringIdentificatorGenerator(IDENTIFICATOR_LENGTH),
			numberOfClients
	);
	this->sessionsDictionary[internalIdentifier] = session;
	this->_activeSession++;
	return session;
}

bool SessionsManager::deleteSession(Session* session) {
	if(this->sessionsDictionary.find(session->internalIdentifier()) == this->sessionsDictionary.end()) {
		return false;
	}
	session->closeSession(0);
	this->sessionsDictionary.erase(session->internalIdentifier());
	delete session;
	this->_activeSession--;
	return true;
}

Session* SessionsManager::findSession(std::string sessionId) {
	for (std::pair<const unsigned int, dataManagerServer::session::Session*> pair : this->sessionsDictionary) {
		if(pair.second->resultIdentifier() == sessionId) {
			return pair.second;
		}
	}
	return nullptr;
}

std::string SessionsManager::sessionStringIdentificatorGenerator(unsigned int length) {
	std::string result(length, '0');
	for (size_t i = 0; i < length; ++i) {
		int randomChar = rand()%(26+10);
		if (randomChar < 26)
			result[i] = 'A' + randomChar;
		else
			result[i] = '0' + randomChar - 26;
	}
	result[length] = 0;

	return result;
}

} /* namespace session */
} /* namespace dataManagerServer */
