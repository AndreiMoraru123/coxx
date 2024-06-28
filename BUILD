cc_binary(
    name = "hello_world",
    srcs = ["main.cxx"],
    copts = ["-std=c++23"],
)

cc_test(
    name = "hello_test",
    size = "small",
    srcs = ["hello_test.cxx"],
    deps = ["@gtest//:gtest_main"],
)
