cc_library(
    name = "common",
    hdrs = ["common.hxx"],
    includes = ["."],
)

cc_test(
    name = "test_socket",
    size = "small",
    srcs = ["test_socket.cxx"],
    copts = ["-std=c++23"],
    deps = [
        ":common",
        "//client:libclient",
        "//server:libserver",
        "@gtest//:gtest_main",
    ],
)

cc_test(
    name = "test_protocol_parsing",
    size = "small",
    srcs = ["test_protocol_parsing.cxx"],
    copts = ["-std=c++23"],
    deps = [
        ":common",
        "//client:libclient",
        "//server:libserver",
        "@gtest//:gtest_main",
    ],
)

cc_test(
    name = "test_event_loop",
    size = "small",
    srcs = ["test_event_loop.cxx"],
    copts = ["-std=c++23"],
    deps = [
        ":common",
        "//client:libclient",
        "//server:libserver",
        "@gtest//:gtest_main",
    ],
)
