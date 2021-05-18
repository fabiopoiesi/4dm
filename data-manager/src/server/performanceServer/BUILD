cc_library(
    name = "TCPConnection",
    srcs = ["TCPConnection.cpp"],
    hdrs = ["TCPConnection.hpp"],
    visibility = ["//visibility:private"],
    deps = [
	"//3rdparty:boost_asio",
	"//3rdparty:boost_shared_ptr",
	"//3rdparty:boost_bind",
	"//3rdparty:boost_enable_shared_from_this",
	"//3rdparty:boost_chrono",
    ],
)

cc_library(
    name = "PerformanceMeasureTCPServer",
    srcs = ["PerformanceMeasureTCPServer.cpp"],
    hdrs = ["PerformanceMeasureTCPServer.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":TCPConnection",
        "//3rdparty:boost_asio",
        "//3rdparty:boost_thread",
        "//3rdparty:boost_bind",
        "//3rdparty:boost_chrono",
    ],
)
