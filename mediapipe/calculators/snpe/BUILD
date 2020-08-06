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
        "//mediapipe/util:resource_util",
        "//third_party:snpe",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
    ],
    alwayslink = 1,
)
