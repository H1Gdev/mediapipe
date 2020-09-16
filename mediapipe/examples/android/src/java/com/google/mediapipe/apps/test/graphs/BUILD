package(default_visibility = ["//mediapipe/examples/android/src/java/com/google/mediapipe/apps/test:__pkg__"])

cc_library(
    name = "mobile_calculators",
    # calculators using in graph.
    deps = [
        "//mediapipe/calculators/test:test_calculator",
        "//mediapipe/calculators/core:constant_side_packet_calculator",
        "//mediapipe/calculators/core:side_packet_to_stream_calculator",
        "//mediapipe/calculators/core:concatenate_vector_calculator",
    ] + [
        # subgraph
        ":test_subgraph",
    ],
)

load(
    "//mediapipe/framework/tool:mediapipe_graph.bzl",
    "mediapipe_binary_graph",
    "mediapipe_simple_subgraph",
)

# convert text graph(*.pbtxt) into binary graph(*.binarypb).
mediapipe_binary_graph(
    name = "mobile_binary_graph",
    graph = "test.pbtxt",
    output_name = "test.binarypb",
    # for node_options type.
    deps = [":mobile_calculators"],
)

# subgraph
mediapipe_simple_subgraph(
    name = "test_subgraph",
    graph = "test_subgraph.pbtxt",
    register_as = "TestSubgraph",
    # calculators using in subgraph.
    deps = [
        "//mediapipe/calculators/core:flow_limiter_calculator",
        "//mediapipe/calculators/core:pass_through_calculator",
        "//mediapipe/calculators/test:test_calculator",
    ],
)
