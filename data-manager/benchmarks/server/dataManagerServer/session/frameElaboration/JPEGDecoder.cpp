#include "benchmark/benchmark.h"

#include "boost/filesystem.hpp"
#include "boost/dll.hpp"

#include "src/server/dataManagerServer/session/frameElaboration/JPEGDecoder.hpp"
#include "src/common/dataManager/frameContainer/Frame.hpp"
#include "src/common/dataManager/frameContainer/FrameDecapsulate.hpp"

#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"

#include <iostream>
#include <string>

#define ADDITIONAL_DATA "aaaaaaaaaa"

#define EXAMPLE_IMAGE_PATH "jpeg_decoder_benchmarks.runfiles/__main__/benchmarks/server/dataManagerServer/session/frameElaboration/example-image.jpg"

dataManagerServer::session::frameElaboration::JPEGDecoder* jpegDecoder;

static void BM_JPEGDecoder(benchmark::State& state) {
	std::string initData = "";
	jpegDecoder = new dataManagerServer::session::frameElaboration::JPEGDecoder();
	jpegDecoder->init(initData);

	bool realImage = true;
	std::cout << "Image path: " << boost::dll::program_location().parent_path() / EXAMPLE_IMAGE_PATH << std::endl;
	// Image encoding
	cv::Mat decodedImage;
	if(realImage) {
		boost::filesystem::path path = boost::dll::program_location().parent_path() / EXAMPLE_IMAGE_PATH;
		decodedImage = cv::imread(path.string(), cv::IMREAD_COLOR);
	} else {
		decodedImage = cv::Mat::zeros(640, 480, CV_8UC3);
	}
	std::vector<uchar> encodedImage = std::vector<uchar>();
	cv::imencode(".jpg", decodedImage, encodedImage);

	for(auto _ : state) {
		std::string* additionalData = new std::string(ADDITIONAL_DATA);
		std::string* frameEncoded = new std::string(encodedImage.begin(), encodedImage.end());
		dataManager::frameContainer::Frame* frame = new dataManager::frameContainer::Frame(0, frameEncoded, additionalData);
		// Image processing
		dataManager::frameContainer::FrameDecapsulate* decapsulate = jpegDecoder->elaborateFrame(frame);
		delete decapsulate;
	}

	delete jpegDecoder;

    state.SetItemsProcessed(state.iterations());
}

BENCHMARK(BM_JPEGDecoder)->Unit(benchmark::kMillisecond);
