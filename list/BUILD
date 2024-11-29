cc_library(
    name = "dlist",
    srcs = ["list.c"],
    hdrs = ["list.h"],
    includes = ["."],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "list",
    hdrs = ["wrap.hxx"],
    includes = ["."],
    visibility = ["//visibility:public"],
    deps = ["dlist"],
)
