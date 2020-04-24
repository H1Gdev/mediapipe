package(default_visibility = ["//mediapipe/examples/android/src/java/com/google/mediapipe/apps/test:__pkg__"])

cc_library(
    name = "mobile_calculators",
    # calculators using in graph.
    deps = [
        "//mediapipe/calculators/test:test_calculator",
    ],
)

load(
    "//mediapipe/framework/tool:mediapipe_graph.bzl",
    "mediapipe_binary_graph",
)

# convert text graph(*.pbtxt) into binary graph(*.binarypb).
mediapipe_binary_graph(
    name = "mobile_binary_graph",
    graph = "test.pbtxt",
    output_name = "test.binarypb",
)
