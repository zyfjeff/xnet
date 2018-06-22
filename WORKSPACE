workspace(name = "xnet")

# We depend on Abseil.
http_archive(
    name = "com_google_absl",
    strip_prefix = "abseil-cpp-master",
    urls = ["https://github.com/abseil/abseil-cpp/archive/master.zip"],
    sha256 = "3ff794abe3f0d7044d892b9516b7afadea40f7d368e0d7783639e45aefdca163",
)

# CCTZ (Time-zone framework). Used by absl.
http_archive(
    name = "com_googlesource_code_cctz",
    strip_prefix = "cctz-2.2",
    urls = ["https://github.com/google/cctz/archive/v2.2.tar.gz"],
    sha256 = "ab315d5beb18a65ace57f6ea91f9ea298ec163fee89f84a44e81732af4d07348",
)

# GoogleTest/GoogleMock framework.
http_archive(
    name = "com_google_googletest",
    strip_prefix = "googletest-master",
    urls = ["https://github.com/google/googletest/archive/master.zip"],
)

http_archive(
  name = "com_google_glog",
  strip_prefix = "glog-master",
  urls = ["https://github.com/google/glog/archive/master.zip"],
)

http_archive(
    name = "com_github_gflags_gflags",
    sha256 = "6e16c8bc91b1310a44f3965e616383dbda48f83e8c1eaa2370a215057b00cabe",
    strip_prefix = "gflags-77592648e3f3be87d6c7123eb81cbad75f9aef5a",
    urls = [
        "https://mirror.bazel.build/github.com/gflags/gflags/archive/77592648e3f3be87d6c7123eb81cbad75f9aef5a.tar.gz",
        "https://github.com/gflags/gflags/archive/77592648e3f3be87d6c7123eb81cbad75f9aef5a.tar.gz",
    ],
)

# Google Benchmark library.
# Adapted from cctz's WORKSPACE.
# Upstream support for bazel is tracked in
#  - https://github.com/google/benchmark/pull/329
#  - https://github.com/google/benchmark/issues/191
new_http_archive(
    name = "com_google_benchmark",
    urls = ["https://github.com/google/benchmark/archive/v1.4.0.tar.gz"],
    strip_prefix = "benchmark-1.4.0",
    sha256 = "616f252f37d61b15037e3c2ef956905baf9c9eecfeab400cb3ad25bae714e214",
    build_file_content =
"""
config_setting(
    name = "windows",
    values = {"cpu": "x64_windows"},
    visibility = [ "//visibility:private" ],
)

cc_library(
    name = "benchmark",
    srcs = glob([
        "src/*.h",
        "src/*.cc",
    ]),
    hdrs = glob(["include/benchmark/*.h"]),
    copts = select({
        "//:windows": ["-DHAVE_STD_REGEX"],
        # HAVE_STD_REGEX didn't work.
        "//conditions:default": ["-DHAVE_POSIX_REGEX"],
    }),
    includes = ["include"],
    visibility = ["//visibility:public"],
)
"""
)