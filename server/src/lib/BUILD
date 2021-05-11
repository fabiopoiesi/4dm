config_setting(
    name = "android_platform",
    values = {"platforms":"android"},
)

cc_binary(
    name = "libimage_sending.so",
    srcs = ["imageSending.cpp","performanceTesting.cpp","imageSending.h","log.h","performanceTesting.h"],
    visibility = ["//visibility:public"],
    deps = [
        "@opencv//:opencv",
        "//src/common/dataManager/frameContainer:frame_decapsulate",
        "//src/common/dataManager/protocol:main_message_cc_proto",
        "//src/common/dataManager/protocol:init_message_cc_proto",
        "//src/common/dataManager/protocol:data_message_cc_proto",
        "//src/common/dataManager/protocol:end_message_cc_proto",
        "@moodycamel_readerwriterqueue//:readerwriterqueue",
    ],
    linkopts=select({
        ":android_platform": ["-llog"],
        "//conditions:default": [],
    }),
    linkshared = 1,
    linkstatic = 1,
)

cc_library(
    name = "image_sending",
    srcs = ["imageSending.cpp","performanceTesting.cpp","imageSending.h","log.h","performanceTesting.h"],
    visibility = ["//visibility:public"],
    deps = [
        "@opencv//:opencv",
        "//src/common/dataManager/frameContainer:frame_decapsulate",
        "//src/common/dataManager/protocol:main_message_cc_proto",
        "//src/common/dataManager/protocol:init_message_cc_proto",
        "//src/common/dataManager/protocol:data_message_cc_proto",
        "//src/common/dataManager/protocol:end_message_cc_proto",
        "@moodycamel_readerwriterqueue//:readerwriterqueue",
        "//src/common/dataManager:md5"
    ],
    copts = [
    	"-std=c++17",
    ],
    linkopts=select({
        ":android_platform": ["-llog"],
        "//conditions:default": [],
    }),
)

