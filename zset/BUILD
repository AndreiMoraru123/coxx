cc_library(
    name = "set",
    srcs = ["zset.cxx"],
    hdrs = ["zset.hxx"],
    copts = ["-std=c++23"],
    visibility = ["//visibility:public"],
    deps = [
        "//avl/c:avl",
        "//map/c:map",
    ],
)

cc_library(
    name = "zset",
    hdrs = ["wrap.hxx"],
    includes = ["."],
    visibility = ["//visibility:public"],
    deps = ["set"],
)
