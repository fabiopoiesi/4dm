cc_test(
	name = "server_integration_tests",
	srcs = ["performanceServer.integ.cpp"],
	deps = [
		"//src/server/performanceServer:PerformanceMeasureTCPServer",
		"//3rdparty:boost_chrono",
		"//3rdparty:boost_thread",
		"@googletest//:gtest_main",
	],
	linkstatic = 1,
)
