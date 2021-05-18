cc_binary(
    name = "performance_server_benchmarks",
    srcs = ["performanceServer.cpp"],
    deps = [
        "//src/server/performanceServer:PerformanceMeasureTCPServer",
		"//3rdparty:boost_chrono",
		"//3rdparty:boost_thread",
		"//3rdparty:boost_asio",
    ],
    linkopts = select({
        "@bazel_tools//src/conditions:darwin": [],
        "@bazel_tools//src/conditions:windows": [],
        "@bazel_tools//src/conditions:linux_x86_64": ["-lpthread"],
    }),
    linkstatic = 1,
)

cc_binary(
    name = "performance_server_benchmarks_bandwidth_control",
    srcs = ["performanceServerBandwidthControl.cpp"],
    deps = [
        "//src/server/performanceServer:PerformanceMeasureTCPServer",
		"//3rdparty:boost_chrono",
		"//3rdparty:boost_thread",
		"//3rdparty:boost_asio",
    ],
    linkopts = select({
        "@bazel_tools//src/conditions:darwin": [],
        "@bazel_tools//src/conditions:windows": [],
        "@bazel_tools//src/conditions:linux_x86_64": ["-lpthread"],
    }),
    linkstatic = 1,
)