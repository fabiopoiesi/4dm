cc_library(
  name = "Decapsulator",
  hdrs = ["Decapsulator.hpp"],
  srcs = ["Decapsulator.cpp"],
  visibility = ["//visibility:public"],
  deps = [
    ":protocolLimit",
    "//src/common:log"
  ]
)

cc_library(
  name = "Encapsulator",
  hdrs = ["Encapsulator.hpp"],
  srcs = ["Encapsulator.cpp"],
  visibility = ["//visibility:public"],
  deps = [
    ":protocolLimit",
    "//src/common:log"
  ]
)

cc_library(
  name = "protocolLimit",
  textual_hdrs = ["protocolLimit.hpp"],
)
