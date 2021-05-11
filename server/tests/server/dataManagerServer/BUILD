cc_test(
    name = "message_parsing_unit_tests",
    srcs = ["MessageParsing.unit.cpp"],
    linkstatic = 1,
    deps = [
        "//3rdparty:boost_system",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "message_decapsulator_integ_tests",
    srcs = ["MessageDecapsulator.integ.cpp"],
    linkstatic = 1,
    deps = [
        "//src/common/dataManager/frameContainer:frame",
        "//src/common/dataManager/protocol:data_message_cc_proto",
        "//src/common/dataManager/protocol:end_message_cc_proto",
        "//src/common/dataManager/protocol:init_message_cc_proto",
        "//src/common/dataManager/protocol:main_message_cc_proto",
        "//src/server/dataManagerServer:message_decapsulator",
        "@googletest//:gtest_main",
        "@opencv",
    ],
)

cc_test(
    name = "data_manager_integ_tests",
    srcs = ["dataManager.integ.cpp"],
    linkstatic = 1,
    deps = [
        "//3rdparty:boost_chrono",
        "//3rdparty:boost_thread",
        "//src/common/dataManager/protocol:data_message_cc_proto",
        "//src/common/dataManager/protocol:end_message_cc_proto",
        "//src/common/dataManager/protocol:init_message_cc_proto",
        "//src/common/dataManager/protocol:main_message_cc_proto",
        "//src/server/dataManagerServer:DataManagerTCPServer",
        "@googletest//:gtest_main",
        "@opencv",
    ],
)
