proto_library(
    name = "snpe_inference_calculator_proto",
    srcs = ["snpe_inference_calculator.proto"],
    visibility = ["//visibility:public"],
    deps = ["//mediapipe/framework:calculator_proto"],
)

load("//mediapipe/framework/port:build_config.bzl", "mediapipe_cc_proto_library")

mediapipe_cc_proto_library(
    name = "snpe_inference_calculator_cc_proto",
    srcs = ["snpe_inference_calculator.proto"],
    cc_deps = ["//mediapipe/framework:calculator_cc_proto"],
    visibility = ["//visibility:public"],
    deps = [":snpe_inference_calculator_proto"],
)

cc_library(
    name = "snpe_inference_calculator",
    srcs = ["snpe_inference_calculator.cc"],
    visibility = ["//visibility:public"],
    deps = [
        ":snpe_inference_calculator_cc_proto",
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/port:status",
    ],
    alwayslink = 1,
)
