cc_library(
    name = "cavl",
    srcs = ["avl.c"],
    hdrs = ["avl.h"],
    includes = ["."],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "avl",
    hdrs = ["wrap.hxx"],
    includes = ["."],
    visibility = ["//visibility:public"],
    deps = ["cavl"],
)
