cc_test(
    name = "jpeg_decoder_unit_tests",
    srcs = ["JPEGDecoder.unit.cpp"],
    deps = [
        "@googletest//:gtest_main",
		"@opencv//:opencv",
        "//src/server/dataManagerServer/session/frameElaboration:jpeg_decoder",
        "//src/common/dataManager/frameContainer:frame",
        "//src/common/dataManager/frameContainer:frame_decapsulate",
    ],
    linkstatic = 1,
)

