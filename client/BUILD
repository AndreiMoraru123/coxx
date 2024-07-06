cc_library(
    name = "client_lib",
    srcs = ["client.cxx"],
    hdrs = ["client.hxx"],
    copts = ["-std=c++23"],
    visibility = ["//visibility:public"],
    deps = [
        "//common:socket",
    ],
)

cc_binary(
    name = "client",
    srcs = ["run.cxx"],
    copts = ["-std=c++23"],
    deps = [":client_lib"],
)
