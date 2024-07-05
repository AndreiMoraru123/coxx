cc_library(
    name = "socket",
    srcs = ["socket.cxx"],
    hdrs = ["socket.hxx"],
    deps = [],
)

cc_library(
    name = "server",
    hdrs = ["server.hxx"],
    copts = ["-std=c++23"],
    deps = [
        ":socket",
    ],
)

cc_library(
    name = "client",
    hdrs = ["client.hxx"],
    copts = ["-std=c++23"],
    deps = [
        ":socket",
    ],
)

cc_test(
    name = "test_one_client.cxx",
    size = "small",
    srcs = ["tests/test_one_client.cxx"],
    copts = ["-std=c++23"],
    deps = [
        ":client",
        ":server",
        ":socket",
        "@gtest//:gtest_main",
    ],
)
