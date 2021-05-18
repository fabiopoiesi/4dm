proto_library(
    name = "main_message_proto",
    srcs = ["MainMessage.proto"],
)

proto_library(
    name = "data_message_proto",
    srcs = ["DataMessage.proto"],
)

proto_library(
    name = "end_message_proto",
    srcs = ["EndMessage.proto"],
)

proto_library(
    name = "init_message_proto",
    srcs = ["InitMessage.proto"],
)

cc_proto_library(
    name = "main_message_cc_proto",
    deps = [":main_message_proto"],
    visibility = [
                "//src/lib:__subpackages__",
		"//src/server/dataManagerServer:__subpackages__",
		"//tests/server/dataManagerServer:__subpackages__",
		"//benchmarks/server/dataManagerServer:__subpackages__",
	],
)

cc_proto_library(
    name = "data_message_cc_proto",
    deps = [":data_message_proto"],
    visibility = [
		"//src/lib:__subpackages__",
		"//src/server/dataManagerServer:__subpackages__",
		"//tests/server/dataManagerServer:__subpackages__",
		"//benchmarks/server/dataManagerServer:__subpackages__",
	],
)

cc_proto_library(
    name = "end_message_cc_proto",
    deps = [":end_message_proto"],
    visibility = [
                "//src/lib:__subpackages__",
		"//src/server/dataManagerServer:__subpackages__",
		"//tests/server/dataManagerServer:__subpackages__",
		"//benchmarks/server/dataManagerServer:__subpackages__",
	],
)

cc_proto_library(
    name = "init_message_cc_proto",
    deps = [":init_message_proto"],
    visibility = [
                "//src/lib:__subpackages__",
		"//src/server/dataManagerServer:__subpackages__",
		"//tests/server/dataManagerServer:__subpackages__",
		"//benchmarks/server/dataManagerServer:__subpackages__",
	],
)
