#include "ServerOrchestrator.hpp"

#include <iostream>

int main() {
	ServerOrchestrator* serverOrchestrator = new ServerOrchestrator();
	std::cout << "Starting server..." << std::endl;
	serverOrchestrator->Start();
	std::cout << "Server started, press ctrl + c for exit" << std::endl;
	serverOrchestrator->WaitStopSignal();
	std::cout << "Stopping server..." << std::endl;
	serverOrchestrator->Stop();
	std::cout << "Server stopped, bye" << std::endl;
	return 0;
}
