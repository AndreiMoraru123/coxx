cc_library(
    name = "cmap",
    srcs = ["map.c"],
    hdrs = ["map.h"],
    includes = ["."],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "map",
    hdrs = ["wrap.hxx"],
    includes = ["."],
    visibility = ["//visibility:public"],
    deps = ["cmap"],
)
