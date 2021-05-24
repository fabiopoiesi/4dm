cc_library(
    name = "abstract_decoder",
    srcs = ["AbstractDecoder.cpp"],
    hdrs = ["AbstractDecoder.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        "@opencv//:opencv",
        "//src/common/dataManager/frameContainer:frame",
        "//src/common/dataManager/frameContainer:frame_decapsulate",
    ],
)

cc_library(
    name = "jpeg_decoder",
    srcs = ["JPEGDecoder.cpp"],
    hdrs = ["JPEGDecoder.hpp"],
    visibility = [
        "//tests/server/dataManagerServer/session/frameElaboration:__pkg__",
        "//benchmarks/server/dataManagerServer/session/frameElaboration:__pkg__",
    ],
    deps = [
        "@opencv//:opencv",
        ":abstract_decoder",
        "//src/common/dataManager/frameContainer:frame",
        "//src/common/dataManager/frameContainer:frame_decapsulate",
    ],
)

cc_library(
    name = "select_decoder",
    srcs = ["SelectDecoder.cpp"],
    hdrs = ["SelectDecoder.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":jpeg_decoder",
        ":abstract_decoder",
        "//src/common/dataManager/protocol:init_message_cc_proto",
        "//3rdparty:boost_bind",
    ],
)
