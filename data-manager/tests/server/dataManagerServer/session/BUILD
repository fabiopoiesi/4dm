cc_test(
    name = "session_integ_tests",
    srcs = ["Session.integ.cpp"],
    deps = [
        "@googletest//:gtest_main",
		"@opencv//:opencv",
		"//src/server/dataManagerServer/session:session",
		"//src/common/dataManager/protocol:init_message_cc_proto",
        "//src/common/dataManager/frameContainer:frame",
        "//src/server/dataManagerServer/session:client",
        "//3rdparty:boost_filesystem",
    ],
    linkstatic = 1,
)