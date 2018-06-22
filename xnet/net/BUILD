load("//:copts.bzl", "DEFAULT_COPTS", "TEST_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "event_loop",
    srcs = [
      "event_loop.h",
      "event_loop.cc",
      "channel.h",
      "channel.cc",
      "poller.h",
      "poller.cc"
    ],
    copts = DEFAULT_COPTS,
    deps = [
      "@com_google_absl//absl/base",
      "@com_google_glog//:glog",
      "//xnet/base:base",
    ]
)

cc_test(
    name = "event_loop_unittests",
    srcs = ["event_loop_unittests.cc"],
    copts = TEST_COPTS,
    deps = [
        ":event_loop",
        "@com_google_googletest//:gtest_main",
    ]
)