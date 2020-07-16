cc_binary(
    name = "snpe_desktop",
    srcs = ["snpe_desktop.cc"],
    data = [
        ":input.float32.raw",
        "//mediapipe/models:model.dlc",
    ],
    deps = [
        "//mediapipe/calculators/snpe:snpe_inference_calculator",
        "//mediapipe/framework:calculator_graph",
        "//mediapipe/framework/port:file_helpers",
        "//mediapipe/framework/port:logging",
        "//mediapipe/framework/port:parse_text_proto",
        "//mediapipe/framework/port:status",
    ],
)
