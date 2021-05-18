/*
 * imageSending.cpp
 *
 *  Created on: May 15, 2019
 *      Author: mbortolon
 */

// #define __DBG_INFO 1
#define __SAVE_AS_FILE_FOR_DBG 1
// #define __CHECK_ACCESS_ERROR 1
// #define __PRINT_FUNCTION_CALL 1

#include "imageSending.h"

#include <queue>
#include <string>
#include <sstream>

#include <vector>

#include <iostream>
#include <fstream>

#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <exception>

#ifdef __SAVE_AS_FILE_FOR_DBG
#include <sys/stat.h>
#endif

// #include <thread>
#include <chrono>
#include <ctime>

#include "src/common/dataManager/frameContainer/FrameDecapsulate.hpp"

#include "src/common/dataManager/protocol/InitMessage.pb.h"
#include "src/common/dataManager/protocol/MainMessage.pb.h"
#include "src/common/dataManager/protocol/DataMessage.pb.h"
#include "src/common/dataManager/protocol/EndMessage.pb.h"

#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"

#include "log.h"

#include "atomicops.h"
#include "readerwriterqueue.h"

#define PACKET_MAX_SIZE 30000
#define APPLICATION_ID "a"
#define INIT_BYTES ""
#define SERVER_MAX_PACKET_SIZE 400000

#define NET_SOFTERROR -1
#define NET_HARDERROR -2

#define DATA_MANAGER_STANDARD_MESSAGE_RESPONSE_CODE 0
#define DATA_MANAGER_TOO_LARGE_MESSAGE_RESPONSE_CODE 1

namespace ImageSending {
	typedef struct {
		int socket = -1;

		uint64_t maximumRate;
		unsigned int sleepClock = 0;

		uint32_t clients = 0;
		std::string streamId = "";
		bool isHost = false;
		int64_t acquiredFrames = -1;

		std::string serverIpAddress = "";
		uint32_t serverPort = 5959;
		std::string phoneId = "";

		moodycamel::BlockingReaderWriterQueue<dataManager::frameContainer::FrameDecapsulate*> imageQueue;

#ifdef __SAVE_AS_FILE_FOR_DBG
		std::ofstream correctEncodeImageFile;
		std::ofstream correctSendFile;

		std::string imageSavePath;
#endif
	} sendingState;

	std::vector<sendingState> container;

	extern "C" unsigned int initSendingImage(uint64_t maximumRate, const char* libDebugFolder) {
#ifdef __PRINT_FUNCTION_CALL
		LOGD("initSendingImage call");
#endif
		container.push_back(sendingState());
		int position = container.size() - 1;
		setMaximumRate(position, maximumRate);
#ifdef __SAVE_AS_FILE_FOR_DBG
		time_t rawtime;
		struct tm * timeinfo;
		char buffer[80];
		time (&rawtime);
		timeinfo = localtime(&rawtime);
		strftime(buffer,sizeof(buffer),"%Y-%m-%d-%H-%M-%S",timeinfo);
		std::string timeStr(buffer);
		std::string libDebugFolderString(libDebugFolder);
		libDebugFolderString = libDebugFolderString.erase(libDebugFolderString.find_last_not_of("\\")+1);
		container[position].correctEncodeImageFile.open(libDebugFolderString + std::string("/") + std::string("correctEncodeImage") + timeStr + std::string(".txt"));
		container[position].correctSendFile.open(libDebugFolderString + std::string("/") + std::string("correctSend") + timeStr + std::string(".txt"));
		container[position].imageSavePath = libDebugFolderString + std::string("/") + std::string("images") + timeStr;
		mkdir(container[position].imageSavePath.c_str(), 0777);
#endif
		return position;
	}

	extern "C" void setSessionClient(unsigned int position, uint32_t clients) {
#ifdef __PRINT_FUNCTION_CALL
		LOGD("setSessionClient call");
#endif

#ifdef __CHECK_ACCESS_ERROR
		if(position >= container.size()) {
			LOGE("Given socket number is not valid");
			return;
		}
#endif
		container[position].isHost = true;
		container[position].streamId = "";
		container[position].clients = clients;
	}

	extern "C" void setSessionIdentifier(unsigned int position, const char* streamId) {
#ifdef __PRINT_FUNCTION_CALL
		LOGD("setSessionIdentifier call");
#endif
#ifdef __CHECK_ACCESS_ERROR
		if(position >= container.size()) {
			LOGE("Given socket number is not valid");
			return;
		}
#endif
		container[position].isHost = false;
		container[position].streamId = std::string(streamId);
		container[position].clients = 0;
	}

	inline void wait(int clockWait)
	{
		clock_t endwait;
		endwait = clock () + clockWait;
		unsigned int waitTime = 0;
		while (clock() < endwait) { waitTime++; }
#ifdef __DBG_INFO
		std::cout << "Waited " << waitTime << " to send the package" << std::endl;
#endif
	}

	char firstPacketBuffer[PACKET_MAX_SIZE] = { 0 };

	bool socketSend(int sock, const char* array, size_t size, unsigned int sleepTimeInClock) {
#ifdef __PRINT_FUNCTION_CALL
		LOGD("socketSend call");
#endif
#ifdef __DBG_INFO
		std::cout << "Send encapsulated array: [";
		for(unsigned int c = 0; c < 10; c++) {
			std::cout << (static_cast<int>(array[c]) & 0xFF) << " ";
		}
		std::cout << "... ";
		for(unsigned int c = size - 10; c < size; c++) {
			std::cout << (static_cast<int>(array[c]) & 0xFF) << " ";
		}
		std::cout << "]" << std::endl;
#endif
		// Don't send the packet if size is too large
		if(size >= SERVER_MAX_PACKET_SIZE) {
			LOGE("Client try to send a too large packet");
			return true;
		}
		uint32_t size_net = htonl((uint32_t) size);
		std::memcpy(firstPacketBuffer, &size_net, sizeof(uint32_t));
		unsigned long int byteToSend = std::min<unsigned long int>(
				PACKET_MAX_SIZE - sizeof(uint32_t), size);
		std::memcpy(&firstPacketBuffer[sizeof(uint32_t)], array, byteToSend);
		wait(sleepTimeInClock);
		size_t sendBytes = send(sock, firstPacketBuffer, byteToSend + sizeof(uint32_t), 0);
		size_t pointer = byteToSend;
		if (sendBytes <= 0) {
			LOGE("Impossible write to the socket, probably it's close");
			return false;
		}

		while (pointer < size) {
			byteToSend = std::min<unsigned long int>(PACKET_MAX_SIZE,
					size - pointer);
			size_t sendBytes = send(sock, &array[pointer], byteToSend, 0);
			wait(sleepTimeInClock);
			pointer += byteToSend;
			if (sendBytes <= 0) {
				std::cout << "Send bytes: " << sendBytes << std::endl;
				return false;
			}
		}
		return true;
	}

	void createInitMessage(dataManager::protocol::InitMessage& initMessage, std::string& phoneId) {
		initMessage.set_applicationid(APPLICATION_ID);
		initMessage.set_phoneid(phoneId);
		initMessage.set_initbytes(INIT_BYTES);
		initMessage.set_contentcodec(dataManager::protocol::InitMessage_ContentCodec::InitMessage_ContentCodec_MJPEG);
	}

	void createInitMessageClient(dataManager::protocol::InitMessage& initMessage, std::string& phoneId, std::string& streamId) {
		createInitMessage(initMessage, phoneId);
		initMessage.set_streamid(streamId);
	}

	void createInitMessageHost(dataManager::protocol::InitMessage& initMessage, std::string& phoneId, u_int32_t numberOfClient) {
		createInitMessage(initMessage, phoneId);
		initMessage.set_clients(static_cast<int32_t>(numberOfClient));
	}

	std::string encapsulateInsideMainMessage(std::string& message, dataManager::protocol::MainMessage_Type messageType) {
		dataManager::protocol::MainMessage mainMessage = dataManager::protocol::MainMessage();
		mainMessage.set_type(messageType);
		mainMessage.set_info(message);
		return mainMessage.SerializeAsString();
	}

	extern "C" void sendingComplete(unsigned int position, int64_t totalFrame) {
#ifdef __PRINT_FUNCTION_CALL
		LOGD("sendingComplete call");
#endif
#ifdef __CHECK_ACCESS_ERROR
		if(position >= container.size()) {
			LOGE("Given socket number is not valid");
			return;
		}
#endif
		container[position].acquiredFrames = totalFrame;
	}

	bool createConnectionInternal(unsigned int position, bool isRejoin) {
#ifdef __PRINT_FUNCTION_CALL
		LOGD("createConnection call");
#endif
#ifdef __CHECK_ACCESS_ERROR
		if(position >= container.size()) {
			LOGE("Given socket number is not valid");
			return false;
		}
#endif
		struct sockaddr_in serv_addr;
		memset(&serv_addr, '0', sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(container[position].serverPort);
		// Convert IPv4 and IPv6 addresses from text to binary form
		inet_pton(AF_INET, container[position].serverIpAddress.c_str(), &serv_addr.sin_addr);

		container[position].socket = socket(AF_INET, SOCK_STREAM, 0);
		int flags = 1;
		setsockopt(container[position].socket, IPPROTO_TCP, TCP_NODELAY, (void*)&flags, sizeof(flags));
		connect(container[position].socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

		if(isRejoin) {
			dataManager::protocol::InitMessage initMessage = dataManager::protocol::InitMessage();
			createInitMessageClient(initMessage, container[position].phoneId, container[position].streamId);
			initMessage.set_rejoin(isRejoin);
			std::string initMessageEncoded = initMessage.SerializeAsString();
			std::string encapsulateInitMessage = encapsulateInsideMainMessage(initMessageEncoded, dataManager::protocol::MainMessage_Type::MainMessage_Type_INIT);

			socketSend(container[position].socket, encapsulateInitMessage.c_str(), encapsulateInitMessage.size(), 0);

			initMessage.Clear();
			initMessageEncoded.clear();
			encapsulateInitMessage.clear();
			return true;
		}

		if(container[position].isHost == true) {// +++ host +++

			dataManager::protocol::InitMessage initMessage = dataManager::protocol::InitMessage();

			createInitMessageHost(initMessage, container[position].phoneId, container[position].clients);
			std::string initMessageEncoded = initMessage.SerializeAsString();

			std::string encapsulateInitMessage = encapsulateInsideMainMessage(initMessageEncoded, dataManager::protocol::MainMessage_Type::MainMessage_Type_INIT);

			socketSend(container[position].socket, encapsulateInitMessage.c_str(), encapsulateInitMessage.size(), 0);

			int count = 0;
			int numberTry = 0;
			while (numberTry < 20 && count == 0) {
				ioctl(container[position].socket, FIONREAD, &count);
				numberTry++;
				usleep(60000);
				// std::this_thread::sleep_for(std::chrono::microseconds(60));
			}
			if(count == 0) {
				LOGE("Server don't respond in a correct way");
				return false;
			}

			char buffer[400] = { 0 };
			read(container[position].socket, buffer, 400);

			dataManager::protocol::InitMessageResponse initMessageResponse =
						dataManager::protocol::InitMessageResponse();
			initMessageResponse.ParseFromArray(buffer + sizeof(uint32_t), count - sizeof(uint32_t));

			container[position].streamId = std::string(initMessageResponse.streamid());

			// Take out the garbage
			initMessage.Clear();
			initMessageEncoded.clear();
			encapsulateInitMessage.clear();
			initMessageResponse.Clear();

		} else {// +++ client +++
			dataManager::protocol::InitMessage initMessage = dataManager::protocol::InitMessage();
			createInitMessageClient(initMessage, container[position].phoneId, container[position].streamId);
			std::string initMessageEncoded = initMessage.SerializeAsString();
			std::string encapsulateInitMessage = encapsulateInsideMainMessage(initMessageEncoded, dataManager::protocol::MainMessage_Type::MainMessage_Type_INIT);

			socketSend(container[position].socket, encapsulateInitMessage.c_str(), encapsulateInitMessage.size(), 0);

			initMessage.Clear();
			initMessageEncoded.clear();
			encapsulateInitMessage.clear();
		}

		return true;
	}

	extern "C" bool createConnection(unsigned int position, const char* serverIpAddress, uint32_t serverPort, const char* phoneId) {
#ifdef __PRINT_FUNCTION_CALL
		LOGD("createConnection call");
#endif
#ifdef __CHECK_ACCESS_ERROR
		if(position >= container.size()) {
			LOGE("Given socket number is not valid");
			return false;
		}
#endif
		container[position].serverIpAddress = std::string(serverIpAddress);
		container[position].serverPort = serverPort;
		container[position].phoneId = std::string(phoneId);
		return createConnectionInternal(position, false);
	}

	// de-allocation responsibility is of caller
	extern "C" char* getStreamId(unsigned int position) {
#ifdef __PRINT_FUNCTION_CALL
		LOGD("getStreamId call");
#endif
#ifdef __CHECK_ACCESS_ERROR
		if(position >= container.size()) {
			LOGE("Given socket number is not valid");
			return nullptr;
		}
#endif
		char* cpStreamId = new char[container[position].streamId.size() + 1];
		strcpy(cpStreamId, container[position].streamId.c_str());
		return cpStreamId;
	}

	void closeSession(unsigned int position) {
		try {
			if(container[position].isHost == true) {
				LOGD("Preparing host end message");
				// Prepare end message
				dataManager::protocol::EndMessage endMessage = dataManager::protocol::EndMessage();
				endMessage.set_totalacquireframes(container[position].acquiredFrames);
				std::string endMessageEncoded = endMessage.SerializeAsString();

				std::string encapsulateEndMessage = encapsulateInsideMainMessage(endMessageEncoded, dataManager::protocol::MainMessage_Type::MainMessage_Type_END);

				socketSend(container[position].socket, encapsulateEndMessage.c_str(), encapsulateEndMessage.size(), 0);
				LOGD("Finish sending end packet");
				encapsulateEndMessage.clear();
				endMessageEncoded.clear();
				endMessage.Clear();
				LOGD("Remove garbage");
			}
			LOGD("Socket id %d", container[position].socket);
			shutdown(container[position].socket, SHUT_RDWR);
			std::cout << "Socket close" << std::endl;
			container[position].socket = -1;
#ifdef __SAVE_AS_FILE_FOR_DBG
			container[position].correctEncodeImageFile.close();
			container[position].correctSendFile.close();
#endif
			//container.erase(container.begin() + position);
			LOGD("Socket close");
		}
		catch (std::exception& e)
		{
			const char* errMsg = e.what();
			LOGE("Exception during socket closing, exception message: %s", errMsg);
		}
		catch(...) {
			LOGE("Not standard exception intercept");
		}
	}

	void closeConnectionWithoutGrace(unsigned int position) {
#ifdef __PRINT_FUNCTION_CALL
		LOGD("closeConnectionWithoutGrace");
#endif
		try {
			LOGD("Socket id %d", container[position].socket);
			close(container[position].socket);
			std::cout << "Socket close" << std::endl;
			LOGD("Socket close");
		}
		catch (std::exception& e)
		{
			const char* errMsg = e.what();
			LOGE("Exception during socket closing, exception message: %s", errMsg);
		}
		catch(...) {
			LOGE("Not standard exception intercept");
		}
	}

	void sessionRestart(unsigned int position) {
#ifdef __PRINT_FUNCTION_CALL
		LOGD("sessionRestart call");
#endif
		closeConnectionWithoutGrace(position);
		createConnectionInternal(position, true);
	}

	void flushingQueue(unsigned int position) {
		bool moreElement = true;
		dataManager::protocol::DataMessage dataMessage = dataManager::protocol::DataMessage();
		do {
			dataManager::frameContainer::FrameDecapsulate* item = nullptr;
			moreElement = container[position].imageQueue.wait_dequeue_timed(item, std::chrono::milliseconds(10));
			if(!moreElement) {
				return;
			}
			std::cout << "Image extract from queue at position " << position << std::endl;
			// Prepare data message
			dataMessage.set_frameid(item->frameId);
			dataMessage.set_additionaldata(*item->additionalData);
			dataMessage.set_data(item->frame->data(), item->frame->size());

#ifdef __DBG_INFO
			unsigned char* dataPointer = item->frame->data();
			std::cout << "Send data array: [";
			for(unsigned int c = 0; c < 10; c++) {
				std::cout << (unsigned short)(dataPointer[c]) << " ";
			}
			std::cout << "... ";
			for(unsigned int c = item->frame->size() - 10; c < item->frame->size(); c++) {
				std::cout << (unsigned short)(dataPointer[c]) << " ";
			}
			std::cout << "]" << std::endl;
#endif

			std::string dataMessageEncoded = dataMessage.SerializeAsString();
			std::string encapsulateDataMessage = encapsulateInsideMainMessage(dataMessageEncoded, dataManager::protocol::MainMessage_Type::MainMessage_Type_DATA);

			bool result = socketSend(container[position].socket, encapsulateDataMessage.c_str(), encapsulateDataMessage.size(), container[position].sleepClock);
			if(result == false) {
				sessionRestart(position);
			}
			std::cout << "Client lib send message of size: " << encapsulateDataMessage.size() << " for connection: " << position << " with result: " << result << std::endl;

			int count;
			ioctl(container[position].socket, FIONREAD, &count);
			if(count != 0) {
				uint32_t messageType = 0;
				read(container[position].socket, &messageType, sizeof(uint32_t));
				messageType = ntohl(messageType);
				if(messageType == DATA_MANAGER_TOO_LARGE_MESSAGE_RESPONSE_CODE) {
					sessionRestart(position);
				} else {
					std::cout << "Unreachable, reached" << std::endl;
				}
				// Re-enqueue the frame remove because too large (we assume that the too large message can derive from a transmission error)
				container[position].imageQueue.enqueue(item);
			}
			else {
#ifdef __SAVE_AS_FILE_FOR_DBG
				container[position].correctSendFile << item->frameId << std::endl;
#endif
				delete item;
			}

			encapsulateDataMessage.clear();
			dataMessageEncoded.clear();
			dataMessage.Clear();
		} while(moreElement);
	}

	extern "C" void sendingImageThreadFuction(unsigned int position) {
#ifdef __PRINT_FUNCTION_CALL
		LOGD("sendingImageThreadFuction call");
#endif
		try {
			if(container[position].socket < 0) {
				LOGE("Socket number is not valid %d", container[position].socket);
				return;
			}

			while(container[position].acquiredFrames == -1) {
				flushingQueue(position);
				usleep(60000);
			}
			flushingQueue(position);
			closeSession(position);
		}
		catch (std::exception& e)
		{
			const char* errMsg = e.what();
			LOGE("Exception during socket closing, exception message: %s", errMsg);
		}
		catch(...) {
			LOGE("Not standard exception intercept");
		}
	}

	extern "C" void setMaximumRate(unsigned int position, uint64_t maximumRate) {
#ifdef __PRINT_FUNCTION_CALL
		LOGD("setMaximumRate call");
#endif
#ifdef __CHECK_ACCESS_ERROR
		if(position >= container.size()) {
			LOGE("Given socket number is not valid");
			return;
		}
#endif
		container[position].maximumRate = maximumRate;
		if(maximumRate == 0) {
			container[position].sleepClock = 0;
			return;
		}
		container[position].sleepClock = (1.0f / (maximumRate / PACKET_MAX_SIZE)) * CLOCKS_PER_SEC;
		std::cout << "sleepClock: " << container[position].sleepClock << std::endl;
	}

	extern "C" bool sendImage(unsigned int position, uint64_t frameId, const unsigned char* y_buffer, const unsigned char* uv_buffer, int width, int height, const char* additionalData, size_t additionalDataLenght) {
#ifdef __PRINT_FUNCTION_CALL
		LOGD("sendImage call");
#endif
#ifdef __CHECK_ACCESS_ERROR
		if(position >= container.size()) {
			LOGE("Given socket number is not valid");
			return false;
		}
#endif
		try {
			// Image encoding to jpg
			LOGD("Library build time: %s", __TIMESTAMP__);

			cv::Mat yMatrix(height, width, CV_8UC1, (void*)y_buffer);
			cv::Mat uvMatrix(height / 2, width / 2, CV_8UC1, (void*)uv_buffer);

			// Matrix reconstruction
			cv::Mat bgrMatrix(height, width, CV_8UC3);

			cv::cvtColorTwoPlane(yMatrix, uvMatrix, bgrMatrix, cv::COLOR_YUV2BGR_NV21);

#ifdef __DBG_INFO
			std::cout << "bgrMatrix = (" << bgrMatrix.rows << ", " << bgrMatrix.cols << ")" << std::endl;
#endif

			std::vector<uchar>* imageBuffer = new std::vector<uchar>();
			bool result = cv::imencode(".jpg", bgrMatrix, *imageBuffer);
			if(result == false) {
				LOGE("Image encode failed");
				delete imageBuffer;
				return false;
			}

#ifdef __SAVE_AS_FILE_FOR_DBG
			std::ostringstream jpegFilename;
			jpegFilename << frameId << ".jpg";
			std::ostringstream binFilename;
			binFilename << frameId << ".bin";
			std::string jpegPath = container[position].imageSavePath + std::string("/") + std::string(jpegFilename.str());
			std::string binPath = container[position].imageSavePath + std::string("/") + std::string(binFilename.str());
			
			std::ofstream textout(jpegPath, std::ios::out | std::ios::binary);
			textout.write((const char*)&(*imageBuffer)[0], imageBuffer->size());
			textout.close();

			std::ofstream binout(binPath, std::ios::out | std::ios::binary);
			binout.write(additionalData, additionalDataLenght);
			binout.close();
#endif

			std::string* additionalDataString = new std::string(additionalData, additionalDataLenght);
			container[position].imageQueue.enqueue(new dataManager::frameContainer::FrameDecapsulate(frameId, imageBuffer, additionalDataString));
#ifdef __SAVE_AS_FILE_FOR_DBG
			container[position].correctEncodeImageFile << frameId << std::endl;
#endif
		}
		catch (cv::Exception& e) {
			const char* errMsg = e.what();
			LOGE("Exception during image processing, exception message: %s", errMsg);
			return false;
		}
		return true;
	}

	extern "C" bool isSendComplete(unsigned int position) {
#ifdef __PRINT_FUNCTION_CALL
		LOGD("isSendComplete call");
#endif
		if(position < container.size()) {
			return true;
		}
		return (container[position].socket == -1);
	}
}


