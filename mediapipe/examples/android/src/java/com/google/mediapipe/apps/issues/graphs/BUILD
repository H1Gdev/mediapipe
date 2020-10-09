package(default_visibility = ["//mediapipe/examples/android/src/java/com/google/mediapipe/apps/issues:__pkg__"])

cc_library(
    name = "calculators",
    deps = [
        "//mediapipe/calculators/core:pass_through_calculator",
        "//mediapipe/calculators/core:side_packet_to_stream_calculator",
        "//mediapipe/calculators/util:local_file_contents_calculator",
    ],
)

load(
    "//mediapipe/framework/tool:mediapipe_graph.bzl",
    "mediapipe_binary_graph",
)

mediapipe_binary_graph(
    name = "mobile_binary_graph",
    graph = "graph.pbtxt",
    output_name = "graph.binarypb",
    deps = [":calculators"],
)

exports_files([
    "labelmap0.txt",
    "labelmap1.txt",
])
