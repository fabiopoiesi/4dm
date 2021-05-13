cc_library(
	name = "message_decapsulator",
	srcs = ["MessageDecapsulator.cpp"],
	hdrs = ["MessageDecapsulator.hpp"],
	visibility = [
		"__pkg__",
		"//tests/server/dataManagerServer:__pkg__",
		"//benchmarks/server/dataManagerServer:__pkg__",
	],
	deps = [
		"//src/server/dataManagerServer/session:sessions_manager",
        "//src/server/dataManagerServer/session:session",
        "//src/server/dataManagerServer/session:client",
        "//src/common/dataManager/frameContainer:frame",
        "//src/common/dataManager/protocol:main_message_cc_proto",
		"//src/common/dataManager/protocol:init_message_cc_proto",
		"//src/common/dataManager/protocol:data_message_cc_proto",
		"//src/common/dataManager/protocol:end_message_cc_proto",
	],
)

cc_library(
    name = "TCPConnection",
    srcs = ["TCPConnection.cpp"],
    hdrs = ["TCPConnection.hpp"],
    visibility = ["//visibility:private"],
    deps = [
    	"//3rdparty:boost_chrono",
    	"//3rdparty:boost_asio",
    	"//3rdparty:boost_shared_ptr",
    	"//3rdparty:boost_bind",
    	"//3rdparty:boost_enable_shared_from_this",
    	":message_decapsulator",
    ],
)

cc_library(
    name = "DataManagerTCPServer",
    srcs = ["DataManagerTCPServer.cpp"],
    hdrs = ["DataManagerTCPServer.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":TCPConnection",
        "//3rdparty:boost_asio",
        "//3rdparty:boost_thread",
    ],
)
