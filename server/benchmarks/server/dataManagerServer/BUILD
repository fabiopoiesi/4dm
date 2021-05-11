cc_binary(
    name = "data_manager_benchmarks",
    srcs = ["DataManager.cpp"],
    deps = [
        "@benchmark//:benchmark_main",
        "//src/server/dataManagerServer:DataManagerTCPServer",
        "@opencv//:opencv",
        "//src/common/dataManager/protocol:main_message_cc_proto",
        "//src/common/dataManager/protocol:init_message_cc_proto",
        "//src/common/dataManager/protocol:data_message_cc_proto",
        "//src/common/dataManager/protocol:end_message_cc_proto",
        "//3rdparty:boost_chrono",
        "//3rdparty:boost_thread",
        "//3rdparty:boost_filesystem",
        "//3rdparty:boost_dll",
    ],
    data = [
        "example-image.jpg",
    ],
    linkopts = ["-lpthread"],
#    linkopts = select({
#        "@bazel_tools//src/platforms:osx": [],
#        "@bazel_tools//src/platforms:windows": [],
#        "@bazel_tools//src/platforms:linux": ["-lpthread"],
#    }),
    linkstatic = 1,
)

cc_binary(
    name = "message_decapsulator_benchmarks",
    srcs = ["MessageDecapsulator.cpp"],
    deps = [
        "@benchmark//:benchmark_main",
        "//src/server/dataManagerServer:message_decapsulator",
        "@opencv//:opencv",
        "//src/common/dataManager/protocol:main_message_cc_proto",
        "//src/common/dataManager/protocol:init_message_cc_proto",
        "//src/common/dataManager/protocol:data_message_cc_proto",
        "//src/common/dataManager/protocol:end_message_cc_proto",
    ],
    linkopts = ["-lpthread"],
#    linkopts = select({
#        "@bazel_tools//src/platforms:osx": [],
#        "@bazel_tools//src/platforms:windows": [],
#        "@bazel_tools//src/platforms:linux": ["-lpthread"],
#    }),
    linkstatic = 1,
)
