load("//:copts.bzl", "DEFAULT_COPTS", "TEST_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "event_loop",
    srcs = [
      "event_loop.h",
      "event_loop.cc",
      "event_loop_thread.h",
      "event_loop_thread.cc",
      "channel.h",
      "channel.cc",
      "poller.h",
      "poller.cc",
      "timer.h",
      "timer.cc",
      "timestamp.h",
      "timer_queue.h",
      "timer_queue.cc",
      "timer_id.h",
      "callbacks.h",
      "acceptor.h",
      "acceptor.cc",
      "tcp_server.h",
      "tcp_server.cc",
      "tcp_connection.h",
      "tcp_connection.cc",
      "buffer.h",
      "buffer.cc",
      "event_loop_thread_pool.h",
      "event_loop_thread_pool.cc",
    ],
    copts = DEFAULT_COPTS,
    deps = [
      ":socket_lib",
      "@com_google_absl//absl/base",
      "@com_google_absl//absl/strings",
      "@com_google_glog//:glog",
      "//xnet/base:base",
    ]
)

cc_library(
    name = "socket_lib",
    srcs = [
        "socket_utils.h", 
        "socket_utils.cc",
        "inet_address.h",
        "inet_address.cc",
        "socket.h",
        "socket.cc",
    ],
    copts = DEFAULT_COPTS,
    deps = [
        "//xnet/base:base",
        "@com_google_glog//:glog",
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

cc_test(
    name = "timer_unittests",
    srcs = ["timer_unittests.cc"],
    copts = TEST_COPTS,
    deps = [
        ":event_loop",
        "@com_google_googletest//:gtest_main",
    ]
)