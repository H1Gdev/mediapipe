load("//mediapipe/framework/port:build_config.bzl", "mediapipe_proto_library")

mediapipe_proto_library(
    name = "test_calculator_proto",
    srcs = ["test_calculator.proto"],
    visibility = ["//visibility:public"],
    deps = ["//mediapipe/framework:calculator_proto"],
)

cc_library(
    name = "test_calculator",
    srcs = ["test_calculator.cc"],
    visibility = ["//visibility:public"],
    deps = [
        ":test_calculator_cc_proto",
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/port:status",
    ],
    alwayslink = 1,
)
