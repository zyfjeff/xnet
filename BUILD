load("//:copts.bzl", "DEFAULT_COPTS", "TEST_COPTS")

config_setting(
    name = "llvm_compiler",
    values = {
        "compiler": "llvm",
    },
    visibility = [
        ":__subpackages__",
    ],
)

config_setting(
    name = "windows",
    values = {
        "cpu": "x64_windows",
    },
    visibility = [
        ":__subpackages__",
    ],
)
