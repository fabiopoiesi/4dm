load("@io_bazel_rules_docker//cc:image.bzl", "cc_image")
load("@io_bazel_rules_docker//container:container.bzl", "container_push")

cc_library(
    name = "ServerOrchestrator",
    srcs = glob(["ServerOrchestrator.cpp"]),
    hdrs = glob(["ServerOrchestrator.hpp"]),
    visibility = ["//visibility:public"],
    deps = [
        "//src/server/performanceServer:PerformanceMeasureTCPServer",
        "//src/server/dataManagerServer:DataManagerTCPServer",
        "//3rdparty:boost_thread",
        "//3rdparty:boost_asio",
    ],
)

cc_binary(
    name = "data-manager",
    srcs = ["data-manager.cpp"],
    deps = [
        ":ServerOrchestrator",
    ],
    linkopts = [
        "-pthread",
    ],
)

cc_image(
    name = "data-manager-image",
    binary = ":data-manager",
    base = "@official_ubuntu//image",
)

container_push(
    name = "data-manager-publish",
    image=":data-manager-image",
    format = "Docker",
    registry = "gitlab-registry.fbk.eu",
    repository = "mbortolon/datamanager",
    tag = "latest",
)
