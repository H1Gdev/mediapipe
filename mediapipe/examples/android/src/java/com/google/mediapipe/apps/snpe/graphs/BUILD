package(default_visibility = ["//mediapipe/examples/android/src/java/com/google/mediapipe/apps/snpe:__pkg__"])

cc_library(
    name = "mobile_calculators",
    deps = [
        "//mediapipe/calculators/core:pass_through_calculator",
        "//mediapipe/calculators/snpe:snpe_inference_calculator",
    ],
)

load(
    "//mediapipe/framework/tool:mediapipe_graph.bzl",
    "mediapipe_binary_graph",
)

mediapipe_binary_graph(
    name = "mobile_binary_graph",
    graph = "snpe.pbtxt",
    output_name = "snpe.binarypb",
    deps = [":mobile_calculators"],
)
