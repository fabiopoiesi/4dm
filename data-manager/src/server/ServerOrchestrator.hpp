#ifndef SRC_SERVERORCHESTRATOR_HPP_
#define SRC_SERVERORCHESTRATOR_HPP_

#include "performanceServer/PerformanceMeasureTCPServer.hpp"
#include "dataManagerServer/DataManagerTCPServer.hpp"

#include "boost/thread.hpp"

class ServerOrchestrator
{
public:
	void WaitStopSignal();

	bool Start();

	void Stop();

private:
	static void StopSignalHandler(int signum);

	performanceServer::PerformanceMeasureTCPServer* performanceServerInstance;
	boost::thread* threadServerPerformance;
	bool threadServerPerformanceStarted = false;

	dataManagerServer::DataManagerTCPServer* dataManagerServerInstance;
	boost::thread* threadDataManagerServer;
	bool threadDataManagerStarted = false;

	static bool serverShutdownRequest;

	int performanceServerThreadFunction();

	int dataManagerTCPServerThreadFunction();
};

#endif
