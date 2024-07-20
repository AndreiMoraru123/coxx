cc_library(
    name = "libclient",
    srcs = ["client.cxx"],
    hdrs = ["client.hxx"],
    copts = ["-std=c++23"],
    visibility = ["//visibility:public"],
    deps = [
        "//common:conn",
        "//common:socket",
    ],
)

cc_binary(
    name = "client",
    srcs = ["run.cxx"],
    copts = ["-std=c++23"],
    deps = [":libclient"],
)
