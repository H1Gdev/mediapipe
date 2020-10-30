package(default_visibility = ["//visibility:private"])

cc_binary(
    name = "libmediapipe_jni.so",
    linkshared = 1,
    linkstatic = 1,
    deps = [
        "//mediapipe/examples/android/src/java/com/google/mediapipe/apps/test/graphs:mobile_calculators",
        "//mediapipe/java/com/google/mediapipe/framework/jni:mediapipe_framework_jni",
    ],
)

cc_library(
    name = "mediapipe_jni_lib",
    srcs = [":libmediapipe_jni.so"],
    alwayslink = 1,
)

# Java
android_binary(
    # target name can be omitted if it has the same name as directory.
    #name = "test_android",
    name = "test",
    srcs = glob(["**/*.java"]),
    assets = [
        "//mediapipe/examples/android/src/java/com/google/mediapipe/apps/test/graphs:test.binarypb",
    ],
    assets_dir = "",
    manifest = "//mediapipe/examples/android/src/java/com/google/mediapipe/apps/basic:AndroidManifest.xml",
    manifest_values = {
        "applicationId": "com.google.mediapipe.apps.test",
        "appName": "Test MediaPipe",
        "mainActivity": ".java.MainActivity",
        "cameraFacingFront": "True",
        "binaryGraphName": "test.binarypb",
        "inputVideoStreamName": "input_video",
        "outputVideoStreamName": "output_video",
        "flipFramesVertically": "True",
        "converterNumBuffers": "2",
    },
    multidex = "native",
    deps = [
        ":mediapipe_jni_lib",
        "//mediapipe/examples/android/src/java/com/google/mediapipe/apps/basic:basic_lib",
        "//mediapipe/framework/formats:detection_java_proto_lite",
        "//mediapipe/java/com/google/mediapipe/framework:android_framework",
    ],
)

# Kotlin
load("@io_bazel_rules_kotlin//kotlin:kotlin.bzl", "kt_android_library")

kt_android_library(
    name = "lib",
    srcs = glob(["**/*.kt"]),
    manifest = "//mediapipe/examples/android/src/java/com/google/mediapipe/apps/basic:AndroidManifest.xml",
    resource_files = glob(["res/**"]),
    deps = [
        "//mediapipe/examples/android/src/java/com/google/mediapipe/apps/basic:basic_lib",
        "//mediapipe/framework/formats:detection_java_proto_lite",
    ],
)

android_binary(
    name = "testkt",
    assets = [
        "//mediapipe/examples/android/src/java/com/google/mediapipe/apps/test/graphs:test.binarypb",
    ],
    assets_dir = "",
    manifest = "//mediapipe/examples/android/src/java/com/google/mediapipe/apps/basic:AndroidManifest.xml",
    manifest_values = {
        "applicationId": "com.google.mediapipe.apps.test",
        "appName": "Test MediaPipe kt",
        "mainActivity": ".kotlin.MainActivity",
        "cameraFacingFront": "True",
        "binaryGraphName": "test.binarypb",
        "inputVideoStreamName": "input_video",
        "outputVideoStreamName": "output_video",
        "flipFramesVertically": "True",
        "converterNumBuffers": "2",
    },
    multidex = "native",
    deps = [
        ":lib",
        ":mediapipe_jni_lib",
    ],
)
