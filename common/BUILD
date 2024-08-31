cc_library(
    name = "socket",
    srcs = ["socket.cxx"],
    hdrs = ["socket.hxx"],
    visibility = ["//visibility:public"],
    deps = [],
)

cc_library(
    name = "req",
    srcs = ["req.cxx"],
    hdrs = ["req.hxx"],
    copts = ["-std=c++23"],
    visibility = ["//visibility:public"],
    deps = ["//map/c:cmap"],
)

cc_library(
    name = "conn",
    srcs = ["conn.cxx"],
    hdrs = ["conn.hxx"],
    copts = ["-std=c++23"],
    visibility = ["//visibility:public"],
    deps = [":req"],
)
