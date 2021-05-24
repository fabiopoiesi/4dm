cc_library(
    name = "client",
    srcs = ["Client.cpp"],
    hdrs = ["Client.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        "@moodycamel_concurrentqueue//:concurrentqueue",
        "//src/common/dataManager/protocol:init_message_cc_proto",
        "//src/common/dataManager/frameContainer:frame",
        "//src/common/dataManager/frameContainer:frame_decapsulate",
        "//src/server/dataManagerServer/session/frameElaboration:abstract_decoder",
        "//src/server/dataManagerServer/session/frameElaboration:select_decoder",
        "//3rdparty:boost_filesystem",
    ],
)

cc_library(
    name = "session",
    srcs = ["Session.cpp"],
    hdrs = ["Session.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        "//src/common/dataManager/protocol:init_message_cc_proto",
        ":client",
        "@moodycamel_concurrentqueue//:concurrentqueue",
        "//3rdparty:boost_thread",
        "//3rdparty:boost_filesystem",
        "//3rdparty:boost_bind",
    ],
)

cc_library(
    name = "sessions_manager",
    srcs = ["SessionsManager.cpp"],
    hdrs = ["SessionsManager.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":session",
    ],
)
