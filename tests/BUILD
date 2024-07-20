cc_library(
    name = "common",
    hdrs = ["common.hxx"],
    includes = ["."],
)

cc_test(
    name = "test_client_server",
    size = "small",
    srcs = ["test_client_server.cxx"],
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
