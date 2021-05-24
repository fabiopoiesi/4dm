cc_test(
	name = "performance_testing_tests",
	srcs = ["performanceTesting.integ.cpp"],
	deps = [
		"//src/server/performanceServer:PerformanceMeasureTCPServer",
		"//3rdparty:boost_chrono",
		"//3rdparty:boost_thread",
		"//3rdparty:boost_asio",
		"//src/lib:image_sending",
		"@googletest//:gtest_main",
	],
	linkstatic = 1,
)

cc_test(
	name = "image_sending_tests",
	srcs = ["imageSending.integ.cpp"],
	deps = [
		"//src/server/dataManagerServer:DataManagerTCPServer",
		"//3rdparty:boost_chrono",
		"//3rdparty:boost_thread",
		"//3rdparty:boost_asio",
		"//src/lib:image_sending",
		"//3rdparty:boost_filesystem",
		"//3rdparty:boost_dll",
		"@opencv//:opencv",
		"@googletest//:gtest_main",
	],
	data = [
		"exampleImage.uv.xml",
		"exampleImage.y.xml",
		"exampleImage.info.txt",
	],
	linkstatic = 1,
)
