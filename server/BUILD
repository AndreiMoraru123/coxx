cc_library(
    name = "libserver",
    srcs = ["server.cxx"],
    hdrs = ["server.hxx"],
    copts = ["-std=c++23"],
    visibility = ["//visibility:public"],
    deps = [
        "//common:conn",
        "//common:socket",
    ],
)

cc_binary(
    name = "server",
    srcs = ["run.cxx"],
    copts = ["-std=c++23"],
    deps = [":libserver"],
)
