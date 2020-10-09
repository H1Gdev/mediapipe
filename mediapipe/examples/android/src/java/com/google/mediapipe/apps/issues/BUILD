package(default_visibility = ["//visibility:private"])

cc_binary(
    name = "libmediapipe_jni.so",
    linkshared = 1,
    linkstatic = 1,
    deps = [
        "//mediapipe/examples/android/src/java/com/google/mediapipe/apps/issues/graphs:calculators",
        "//mediapipe/java/com/google/mediapipe/framework/jni:mediapipe_framework_jni",
    ],
)

cc_library(
    name = "mediapipe_jni_lib",
    srcs = [":libmediapipe_jni.so"],
    alwayslink = 1,
)

android_binary(
    name = "issues",
    srcs = glob(["*.java"]),
    assets = [
        "//mediapipe/examples/android/src/java/com/google/mediapipe/apps/issues/graphs:graph.binarypb",
        "//mediapipe/examples/android/src/java/com/google/mediapipe/apps/issues/graphs:labelmap0.txt",
        "//mediapipe/examples/android/src/java/com/google/mediapipe/apps/issues/graphs:labelmap1.txt",
    ],
    assets_dir = "",
    manifest = "//mediapipe/examples/android/src/java/com/google/mediapipe/apps/basic:AndroidManifest.xml",
    manifest_values = {
        "applicationId": "com.google.mediapipe.apps.issues",
        "appName": "Test Issues",
        "mainActivity": ".MainActivity",
        "cameraFacingFront": "True",
        "binaryGraphName": "graph.binarypb",
        "inputVideoStreamName": "input_video",
        "outputVideoStreamName": "output_video",
        "flipFramesVertically": "True",
        "converterNumBuffers": "2",
    },
    multidex = "native",
    deps = [
        ":mediapipe_jni_lib",
        "//mediapipe/examples/android/src/java/com/google/mediapipe/apps/basic:basic_lib",
        "//mediapipe/java/com/google/mediapipe/framework:android_framework",
    ],
)
