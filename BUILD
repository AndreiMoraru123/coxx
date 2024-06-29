cc_library(
    name = "socket",
    srcs = ["socket.cxx"],
    hdrs = ["socket.hxx"],
    deps = [],
)

cc_binary(
    name = "server",
    srcs = ["server.cxx"],
    copts = ["-std=c++23"],
    deps = [
        ":socket",
    ],
)

cc_test(
    name = "hello_test",
    size = "small",
    srcs = ["hello_test.cxx"],
    deps = ["@gtest//:gtest_main"],
)
