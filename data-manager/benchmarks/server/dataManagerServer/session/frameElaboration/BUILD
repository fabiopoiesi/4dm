cc_binary(
    name = "jpeg_decoder_benchmarks",
    srcs = ["JPEGDecoder.cpp"],
    deps = [
        "@benchmark//:benchmark_main",
        "@opencv//:opencv",
        "//src/server/dataManagerServer/session/frameElaboration:jpeg_decoder",
        "//src/common/dataManager/frameContainer:frame",
        "//src/common/dataManager/frameContainer:frame_decapsulate",
        "//3rdparty:boost_filesystem",
        "//3rdparty:boost_dll",
    ],
    linkopts = ["-lpthread"],
#    linkopts = select({
#        "//tools/cc_target_os:darwin": [],
#        "//tools/cc_target_os:windows": [],
#        "//tools/cc_target_os:linux": ["-lpthread"],
#    }),
    linkstatic = 1,
    data = [
        "example-image.jpg",
    ],
)

