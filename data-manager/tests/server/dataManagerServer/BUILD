cc_test(
    name = "message_parsing_unit_tests",
    srcs = ["MessageParsing.unit.cpp"],
    deps = [
        "@googletest//:gtest_main",
        "//3rdparty:boost_system",
    ],
    linkstatic = 1,
)

cc_test(
	name = "message_decapsulator_integ_tests",
	srcs = ["MessageDecapsulator.integ.cpp"],
	deps = [
		"@googletest//:gtest_main",
		"@opencv//:opencv",
		"//src/server/dataManagerServer:message_decapsulator",
		"//src/common/dataManager/frameContainer:frame",
		"//src/common/dataManager/protocol:main_message_cc_proto",
		"//src/common/dataManager/protocol:init_message_cc_proto",
		"//src/common/dataManager/protocol:data_message_cc_proto",
		"//src/common/dataManager/protocol:end_message_cc_proto",
	],
	linkstatic = 1,
)

cc_test(
	name = "data_manager_integ_tests",
	srcs = ["dataManager.integ.cpp"],
	deps = [
		"@googletest//:gtest_main",
		"@opencv//:opencv",
		"//3rdparty:boost_chrono",
		"//3rdparty:boost_thread",
		"//src/common/dataManager/protocol:main_message_cc_proto",
		"//src/common/dataManager/protocol:init_message_cc_proto",
		"//src/common/dataManager/protocol:data_message_cc_proto",
		"//src/common/dataManager/protocol:end_message_cc_proto",
		"//src/server/dataManagerServer:DataManagerTCPServer",
	],
	linkstatic = 1,
)