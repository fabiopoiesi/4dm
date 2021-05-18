/*
 * ServerOrchestrator.integ.cpp
 *
 *  Created on: May 15, 2019
 *      Author: mbortolon
 */

#include "gtest/gtest.h"
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

#include "src/server/ServerOrchestrator.hpp"

#define PORT_DATAMANAGERSERVER 5960
#define PORT_PERFORMANCESERVER 5959

TEST(PerformanceServer, TestCalculationCorrectness){
	ServerOrchestrator* serversController = new ServerOrchestrator();
	serversController->Start();

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));

	int sd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	int rval = 0;

	// DataManagerServer
	EXPECT_NE(sd, -1) << "Impossible create the socket";
	if(sd != -1) {
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(PORT_DATAMANAGERSERVER);

		inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

		rval = connect(sd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

		EXPECT_NE(rval, -1);
		if(rval != -1) {
			close(sd);
		}
	}

	// Performance Server
	sd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	rval = 0;
	EXPECT_NE(sd, -1) << "Impossible create the socket";
	if(sd != -1) {
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(PORT_PERFORMANCESERVER);

		inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

		rval = connect(sd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

		EXPECT_NE(rval, -1);
		if(rval != -1) {
			close(sd);
		}
	}

	serversController->Stop();
}


