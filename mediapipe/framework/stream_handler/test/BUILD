load("//mediapipe/framework/port:build_config.bzl", "mediapipe_cc_proto_library")

# input
mediapipe_cc_proto_library(
    name = "test_input_stream_handler_proto",
    srcs = ["test_input_stream_handler.proto"],
    # visibility is not public, so create stream handler on this path.
    deps = ["//mediapipe/framework:mediapipe_options_proto"],
    cc_deps = ["//mediapipe/framework:mediapipe_options_cc_proto"],
)

cc_library(
    name = "test_input_stream_handler",
    srcs = ["test_input_stream_handler.cc",],
    visibility = ["//visibility:public"],
    deps = [
        ":test_input_stream_handler_proto",
        "//mediapipe/framework:input_stream_handler",
        "//mediapipe/framework/port:logging",
    ],
    alwayslink = 1,
)

# output
mediapipe_cc_proto_library(
    name = "test_output_stream_handler_proto",
    srcs = ["test_output_stream_handler.proto"],
    deps = ["//mediapipe/framework:mediapipe_options_proto"],
    cc_deps = ["//mediapipe/framework:mediapipe_options_cc_proto"],
)

cc_library(
    name = "test_output_stream_handler",
    srcs = [
        "test_output_stream_handler.cc",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":test_output_stream_handler_proto",
        "//mediapipe/framework:output_stream_handler",
        "//mediapipe/framework/port:logging",
    ],
    alwayslink = 1,
)
