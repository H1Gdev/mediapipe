cc_binary(
    name = "test_desktop",
    srcs = ["test_desktop.cc"],
    deps = [
        "//mediapipe/calculators/test:test_calculator",
        "//mediapipe/framework:calculator_graph",
        "//mediapipe/framework/port:logging",
        "//mediapipe/framework/port:parse_text_proto",
        "//mediapipe/framework/port:status",
    ] + [
        "//mediapipe/framework/stream_handler/test:test_input_stream_handler",
        "//mediapipe/framework/stream_handler/test:test_output_stream_handler",
    ],
)
