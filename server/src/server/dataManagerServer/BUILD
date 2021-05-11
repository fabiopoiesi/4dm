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
		"//src/common/dataManager:md5",
	],
)

cc_library(
    name = "TCPConnection",
    srcs = ["TCPConnection.cpp"],
    hdrs = ["TCPConnection.hpp"],
    visibility = ["//visibility:private"],
    deps = [
    	"@muduo//muduo/net",
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
        "@muduo//muduo/net",
        "//src/server/dataManagerServer/session:sessions_manager",
    ],
)
