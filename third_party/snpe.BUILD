[cc_library(
    name = "libsnpe_" + arch[0],
    srcs = glob(
        ["lib/" + arch[1] + "/*.so"],
        exclude = [
            "lib/**/*_system.so",
        ],
    ),
    hdrs = glob(["include/zdl/**/*.hpp"]),
    includes = ["include/zdl"],
    visibility = ["//visibility:public"],
    alwayslink = 1,
) for arch in [
    # glob() not allow empty string.
    ["arm64-v8a", "aarch64-android-clang6.0"],
    ["armeabi-v7a", "arm-android-clang6.0"],
    ["x86_64", "x86_64-linux-clang"],
]]

cc_library(
    name = "libsnpe_dsp",
    srcs = glob(["lib/dsp/*.so"]),
    visibility = ["//visibility:public"],
    alwayslink = 1,
)
