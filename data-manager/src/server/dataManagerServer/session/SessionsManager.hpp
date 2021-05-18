/*
 * SessionsManager.hpp
 *
 *  Created on: Apr 23, 2019
 *      Author: mbortolon
 */

#ifndef SRC_DATAMANAGERSERVER_SESSION_SESSIONSMANAGER_HPP_
#define SRC_DATAMANAGERSERVER_SESSION_SESSIONSMANAGER_HPP_

#include <unordered_map>
#include <string>

#include "Session.hpp"

namespace dataManagerServer {
namespace session {

class SessionsManager {
public:
	static SessionsManager* getInstance();

	Session* createNewSession(unsigned int numberOfClients);
	Session* findSession(std::string sessionId);
	bool deleteSession(Session* session);

	const unsigned int activeSession() const { return _activeSession; }
private:
	static SessionsManager* instance;
	SessionsManager();
	virtual ~SessionsManager();

	std::unordered_map<unsigned int, Session*> sessionsDictionary;

	std::string sessionStringIdentificatorGenerator(unsigned int length);

	std::atomic<unsigned int> _activeSession;
};

} /* namespace session */
} /* namespace dataManagerServer */

#endif /* SRC_DATAMANAGERSERVER_SESSION_SESSIONSMANAGER_HPP_ */
