/*
 * dataManager.integ.cpp
 *
 *  Created on: May 9, 2019
 *      Author: mbortolon
 */

#include "gtest/gtest.h"

#include <string>
#include <atomic>

#include <iostream>
#include <fstream>

#include "boost/chrono.hpp"
#include "boost/thread/thread.hpp"
#include "src/server/dataManagerServer/DataManagerTCPServer.hpp"
#include "boost/filesystem.hpp"
#include "boost/dll.hpp"

#include "src/lib/imageSending.h"

#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"

#include "muduo/net/EventLoop.h"
#include "muduo/net/InetAddress.h"

#define PORT 5960

#define APPLICATION_ID "aaaaaaa"
#define INIT_BYTES ""

#define ADDITIONAL_DATA "additional_data"

#define MAXIMUM_RATE 2000000

#define HOST_PHONEID "fr58a1cre58a1a2wqca"
#define CLIENT_PHONEID "afwe45821sqfr1487cd"

#define EXAMPLE_IMAGE_INFO_PATH "image_sending_tests.runfiles/__main__/tests/lib/exampleImage.info.txt"
#define EXAMPLE_IMAGE_Y_PATH "image_sending_tests.runfiles/__main__/tests/lib/exampleImage.y.xml"
#define EXAMPLE_IMAGE_UV_PATH "image_sending_tests.runfiles/__main__/tests/lib/exampleImage.uv.xml"

dataManagerServer::DataManagerTCPServer* dataManagerServerInstance;
bool serverRun = false;

std::atomic<int> activeSendThread(0);

int dataManagerServerThreadFunction() {
	muduo::net::EventLoop loop;
	muduo::net::InetAddress listenAddr(PORT);
	dataManagerServerInstance = new dataManagerServer::DataManagerTCPServer(&loop, listenAddr);
	serverRun = true;
	loop.loop();
	return 0;
}

int sendImageThreadFunction(unsigned int identifier) {
	ImageSending::sendingImageThreadFuction(identifier);
	activeSendThread++;
	return 0;
}

TEST(DataManagerServer, TestDataManagerCorrectness) {
	boost::thread* threadServerPerformance = new boost::thread(
					&dataManagerServerThreadFunction);

		// Wait for server ready
		int maxTry = 10;
		while (maxTry >= 0 && serverRun == false) {
			boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
			maxTry--;
		}

		boost::filesystem::path hostTmp = boost::filesystem::path("/tmp") / boost::filesystem::path(HOST_PHONEID);
		boost::filesystem::create_directory(hostTmp);
		unsigned int sessionIdentifierHost = ImageSending::initSendingImage(MAXIMUM_RATE, hostTmp.c_str());
		

		ImageSending::setSessionClient(sessionIdentifierHost, 2);

		bool result = ImageSending::createConnection(sessionIdentifierHost, "127.0.0.1", PORT, HOST_PHONEID);
		ASSERT_TRUE(result);

		char* streamIdPointer = ImageSending::getStreamId(sessionIdentifierHost);
		std::string streamId(streamIdPointer);
		delete[] streamIdPointer;

		std::cout << "streamId: " << streamId << std::endl;

		boost::filesystem::path clientTmp = boost::filesystem::path("/tmp") / boost::filesystem::path(CLIENT_PHONEID);
		boost::filesystem::create_directory(clientTmp);
		unsigned int sessionIdentifierClient = ImageSending::initSendingImage(MAXIMUM_RATE, clientTmp.c_str());

		ImageSending::setSessionIdentifier(sessionIdentifierClient, streamId.c_str());

		result = ImageSending::createConnection(sessionIdentifierClient, "127.0.0.1", PORT, CLIENT_PHONEID);
		ASSERT_TRUE(result);

		boost::thread* threadHostImageSending = new boost::thread(
						&sendImageThreadFunction, sessionIdentifierHost);

		boost::thread* threadClientImageSending = new boost::thread(
						&sendImageThreadFunction, sessionIdentifierClient);

		// Wait both image sending thread are active
		maxTry = 10;
		while (maxTry >= 0 && activeSendThread >= 2) {
			boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
			maxTry--;
		}

		// Image encoding

		int width = 0;
		int height = 0;

		boost::filesystem::path exampleImageInfoPath = boost::dll::program_location().parent_path() / EXAMPLE_IMAGE_INFO_PATH;
		std::ifstream infoFile;
		infoFile.open(exampleImageInfoPath.string());
		infoFile >> width >> height;
		infoFile.close();

		cv::Mat yMatrix;
		cv::Mat uvMatrix;

		boost::filesystem::path exampleImageYPath = boost::dll::program_location().parent_path() / EXAMPLE_IMAGE_Y_PATH;
		cv::FileStorage imageYMatrixFile(exampleImageYPath.string(), cv::FileStorage::READ);
		imageYMatrixFile["yBuffer"] >> yMatrix;
		imageYMatrixFile.release();

		boost::filesystem::path exampleImageUVPath = boost::dll::program_location().parent_path() / EXAMPLE_IMAGE_UV_PATH;
		cv::FileStorage imageUVMatrixFile(exampleImageUVPath.string(), cv::FileStorage::READ);
		imageUVMatrixFile["uvBuffer"] >> uvMatrix;
		imageUVMatrixFile.release();

		unsigned char* y_buffer = yMatrix.data;
		unsigned char* uv_buffer = uvMatrix.data;
		char* additionalData = new char[10]{'{','}'};

		ImageSending::sendImage(sessionIdentifierClient, 0, y_buffer, uv_buffer, width, height, additionalData, 10);

		std::cout << "Host sending" << std::endl;

		ImageSending::sendImage(sessionIdentifierHost, 0, y_buffer, uv_buffer, width, height, additionalData, 10);

		delete[] additionalData;
		yMatrix.release();
		uvMatrix.release();

		boost::this_thread::sleep_for(boost::chrono::milliseconds(300));

		std::cout << "Thread complete: " << ImageSending::isSendComplete(sessionIdentifierClient) << std::endl;

		ImageSending::sendingComplete(sessionIdentifierHost, 0);
		ImageSending::sendingComplete(sessionIdentifierClient, 0);

		// Wait for connection close
		maxTry = 100;
		while (maxTry >= 0 && (!ImageSending::isSendComplete(sessionIdentifierClient) || !ImageSending::isSendComplete(sessionIdentifierHost))) {
			std::cout << "Wait sending complete" << std::endl;
			boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
			maxTry--;
		}

		threadHostImageSending->interrupt();
		threadClientImageSending->interrupt();
		delete threadHostImageSending;
		delete threadClientImageSending;

		// Stop the server
		dataManagerServerInstance->Stop();
		threadServerPerformance->interrupt();
		delete threadServerPerformance;

		// Check if files exist

		std::ostringstream resultFolderStream;
		resultFolderStream << "/tmp/uploadedImages/" << streamId << "/";
		std::string resultFolder = resultFolderStream.str();

		std::ostringstream phoneOneResultFolderStream;
		phoneOneResultFolderStream << resultFolder << HOST_PHONEID << "/";
		std::string phoneOneResultFolder = phoneOneResultFolderStream.str();

		std::ostringstream phoneTwoResultFolderStream;
		phoneTwoResultFolderStream << resultFolder << CLIENT_PHONEID << "/";
		std::string phoneTwoResultFolder = phoneTwoResultFolderStream.str();

		ASSERT_TRUE(boost::filesystem::exists(phoneOneResultFolder));
		ASSERT_TRUE(boost::filesystem::exists(phoneTwoResultFolder));

		std::ostringstream phoneOneImageStream;
		phoneOneImageStream << phoneOneResultFolder << "00000.jpg";
		std::string phoneOneImage = phoneOneImageStream.str();

		std::ostringstream phoneOneBinStream;
		phoneOneBinStream << phoneOneResultFolder << "00000.bin";
		std::string phoneOneBin = phoneOneBinStream.str();

		std::ostringstream phoneTwoImageStream;
		phoneTwoImageStream << phoneTwoResultFolder << "00000.jpg";
		std::string phoneTwoImage = phoneTwoImageStream.str();

		std::ostringstream phoneTwoBinStream;
		phoneTwoBinStream << phoneTwoResultFolder << "00000.bin";
		std::string phoneTwoBin = phoneTwoBinStream.str();

		ASSERT_TRUE(boost::filesystem::exists(phoneOneImage));
		ASSERT_TRUE(boost::filesystem::exists(phoneTwoImage));

		cv::Mat imageOne = cv::imread(phoneOneImage, cv::IMREAD_COLOR);
		EXPECT_FALSE(imageOne.data == NULL);
		imageOne.release();
		cv::Mat imageTwo = cv::imread(phoneTwoImage, cv::IMREAD_COLOR);
		EXPECT_FALSE(imageTwo.data == NULL);
		imageTwo.release();

		EXPECT_TRUE(boost::filesystem::exists(phoneOneBin));
		EXPECT_TRUE(boost::filesystem::exists(phoneTwoBin));

		boost::filesystem::remove_all(resultFolder);


}
