#cc_library(
#  name = "boost_asio",
#  srcs = glob(["boost/include/boost/asio/**/*.hpp","boost/include/boost/asio.hpp"]),
#  hdrs = glob(["boost/include/boost/asio/**/*.hpp","boost/include/boost/asio.hpp"]),
#)

cc_library(
  name = "boost_asio",
  textual_hdrs = glob(["boost/include/boost/asio/**/*.hpp","boost/include/boost/asio/**/*.ipp","boost/include/boost/asio.hpp"]),
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
  deps = [
    ":boost_version",
    ":boost_config",
    ":boost_system",
    ":boost_cstdint",
    ":boost_cerrno",
    ":boost_throw_exception",
    ":boost_assert",
    ":boost_limits",
    ":boost_date_time",
    ":boost_bind",
    ":boost_is_placeholder",
    ":boost_regex_fwd",
    ":boost_regex",
  ],
)

cc_library(
  name = "boost_config",
  textual_hdrs = glob(["boost/include/boost/config/**/*.hpp","boost/include/boost/config.hpp"]),
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "boost_version",
  textual_hdrs = ["boost/include/boost/version.hpp"],
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
)

#cc_import(
#  name = "boost_system_lib",
#  hdrs = glob(["boost/include/boost/system/**/*.hpp","boost/include/boost/system.hpp"]),
#  static_library = "boost/lib/libboost_system.a",
#  # If alwayslink is turned on,
#  # libmylib.a will be forcely linked into any binary that depends on it.
#  # alwayslink = 1,
#  visibility = ["//visibility:public"],
#)

cc_library(
  name = "boost_system",
  hdrs = glob(["boost/include/boost/system/**/*.hpp","boost/include/boost/system.hpp"]),
  srcs = ["boost/lib/libboost_system.a"],
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
  deps = [
    ":boost_cerrno",
    ":boost_cstdint",
    ":boost_config",
  ],
)

cc_library(
  name = "boost_cstdint",
  textual_hdrs = ["boost/include/boost/cstdint.hpp"],
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "boost_cerrno",
  textual_hdrs = ["boost/include/boost/cerrno.hpp"],
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "boost_throw_exception",
  textual_hdrs = ["boost/include/boost/throw_exception.hpp"],
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
  deps = [
    ":boost_detail",
    ":boost_exception",
    ":boost_current_function",
  ],
)

cc_library(
  name = "boost_detail",
  textual_hdrs = glob(["boost/include/boost/detail/**/*.hpp"]),
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "boost_exception",
  textual_hdrs = glob(["boost/include/boost/exception/**/*.hpp","boost/include/boost/exception.hpp"]),
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "boost_current_function",
  textual_hdrs = glob(["boost/include/boost/current_function/**/*.hpp","boost/include/boost/current_function.hpp"]),
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "boost_assert",
  textual_hdrs = ["boost/include/boost/assert.hpp"],
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "boost_limits",
  textual_hdrs = ["boost/include/boost/limits.hpp"],
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
)

#cc_import(
#  name = "boost_date_time_lib",
#  hdrs = glob(["boost/include/boost/date_time/**/*.hpp","boost/include/boost/date_time/**/*.ipp","boost/include/boost/date_time.hpp"]),
#  static_library = ,
#  # If alwayslink is turned on,
#  # libmylib.a will be forcely linked into any binary that depends on it.
#  # alwayslink = 1,
#  visibility = ["//visibility:private"],
#)

cc_library(
  name = "boost_date_time",
  hdrs = glob(["boost/include/boost/date_time/**/*.hpp","boost/include/boost/date_time/**/*.ipp","boost/include/boost/date_time.hpp"]),
  srcs = ["boost/lib/libboost_date_time.a"],
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
  deps = [
    ":boost_shared_ptr",
    ":boost_operators",
    ":boost_static_assert",
    ":boost_type_traits",
    ":boost_mpl",
    ":boost_numeric",
    ":boost_limits",
  ],
)

cc_library(
  name = "boost_shared_ptr",
  textual_hdrs = ["boost/include/boost/shared_ptr.hpp"],
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
  deps = [
    ":boost_smart_ptr",
  ],
)

cc_library(
  name = "boost_smart_ptr",
  textual_hdrs = glob(["boost/include/boost/smart_ptr/**/*.hpp","boost/include/boost/smart_ptr.hpp"]),
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
  deps = [
    ":boost_checked_delete",
    ":boost_predef",
  ],
)

cc_library(
  name = "boost_checked_delete",
  textual_hdrs = glob(["boost/include/boost/checked_delete/**/*.hpp","boost/include/boost/checked_delete.hpp"]),
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
  deps = [
    ":boost_core",
  ],
)

cc_library(
  name = "boost_core",
  textual_hdrs = glob(["boost/include/boost/core/**/*.hpp"]),
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "boost_predef",
  textual_hdrs = glob(["boost/include/boost/predef/**/*.h","boost/include/boost/predef.h"]),
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "boost_operators",
  textual_hdrs = ["boost/include/boost/operators.hpp"],
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "boost_static_assert",
  textual_hdrs = ["boost/include/boost/static_assert.hpp"],
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "boost_type_traits",
  textual_hdrs = glob(["boost/include/boost/type_traits/**/*.hpp","boost/include/boost/type_traits.hpp"]),
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
  deps = [
    ":boost_version",
    ":boost_static_assert",
    ":boost_detail",
  ],
)

cc_library(
  name = "boost_mpl",
  textual_hdrs = glob(["boost/include/boost/mpl/**/*.hpp"]),
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
  deps = [
    ":boost_preprocessor",
    ":boost_detail",
  ]
)

cc_library(
  name = "boost_preprocessor",
  textual_hdrs = glob(["boost/include/boost/preprocessor/**/*.hpp"]),
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "boost_numeric",
  textual_hdrs = glob(["boost/include/boost/numeric/**/*.hpp"]),
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
  deps = [
    "boost_type"
  ],
)

cc_library(
  name = "boost_type",
  textual_hdrs = ["boost/include/boost/type.hpp"],
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "boost_bind",
  textual_hdrs = glob(
    include = ["boost/include/boost/bind/**/*.hpp","boost/include/boost/bind.hpp"],
    exclude = ["boost/include/boost/bind/mem_fn.hpp"]
  ),
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
  deps = [
    ":boost_ref",
    ":boost_mem_fn",
    ":boost_bind_mem_fn",
    ":boost_visit_each",
  ]
)

cc_library(
  name = "boost_is_placeholder",
  textual_hdrs = ["boost/include/boost/is_placeholder.hpp"],
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "boost_regex_fwd",
  textual_hdrs = ["boost/include/boost/regex_fwd.hpp"],
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
)

cc_import(
  name = "boost_regex",
  hdrs = glob(["boost/include/boost/regex/**/*.hpp","boost/include/boost/regex/**/*.ipp","boost/include/boost/regex.hpp"]),
  static_library = "boost/lib/libboost_regex.a",
  # If alwayslink is turned on,
  # libmylib.a will be forcely linked into any binary that depends on it.
  # alwayslink = 1,
  visibility = ["//visibility:public"],
)

cc_library(
  name = "boost_enable_shared_from_this",
  textual_hdrs = ["boost/include/boost/enable_shared_from_this.hpp"],
  deps = [
    ":boost_smart_ptr",
  ],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "boost_ref",
  textual_hdrs = ["boost/include/boost/ref.hpp"],
  deps = [
    ":boost_core",
  ],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "boost_mem_fn",
  textual_hdrs = ["boost/include/boost/mem_fn.hpp"],
  deps = [
    ":boost_bind_mem_fn",
  ],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "boost_bind_mem_fn",
  textual_hdrs = ["boost/include/boost/bind/mem_fn.hpp"],
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
  deps = [
    ":boost_get_pointer",
  ]
)

cc_library(
  name = "boost_get_pointer",
  textual_hdrs = ["boost/include/boost/get_pointer.hpp"],
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
  deps = [
    ":boost_config",
  ],
)

cc_library(
  name = "boost_visit_each",
  textual_hdrs = ["boost/include/boost/visit_each.hpp"],
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "boost_lockfree",
  textual_hdrs = glob(["boost/include/boost/lockfree/**/*.hpp"]),
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
  deps = [
    ":boost_aligned_storage",
    ":boost_assert",
    ":boost_utility",
    ":boost_mpl",
    ":boost_array",
    ":boost_parameter",
    ":boost_cstdint",
    ":boost_noncopyable",
    ":boost_align",
    ":boost_predef",
  ]
)

cc_library(
  name = "boost_aligned_storage",
  textual_hdrs = glob(["boost/include/boost/lockfree/**/*.hpp", "boost/include/boost/aligned_storage.hpp"]),
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
  deps = [
    ":boost_type_traits",
  ]
)

cc_library(
  name = "boost_array",
  textual_hdrs = ["boost/include/boost/array.hpp"],
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
  deps = [
    ":boost_swap",
    ":boost_throw_exception",
  ],
)

cc_library(
  name = "boost_parameter",
  textual_hdrs = glob(["boost/include/boost/parameter/**/*.hpp", "boost/include/boost/parameter.hpp"]),
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "boost_swap",
  textual_hdrs = ["boost/include/boost/swap.hpp"],
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "boost_noncopyable",
  textual_hdrs = ["boost/include/boost/noncopyable.hpp"],
)

cc_library(
  name = "boost_align",
  textual_hdrs = glob(["boost/include/boost/align/**/*.hpp", "boost/include/boost/align.hpp"]),
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
)

#cc_import(
#  name = "boost_thread_lib",
#  hdrs = glob(["boost/include/boost/thread/**/*.hpp","boost/include/boost/thread/**/*.ipp","boost/include/boost/thread.hpp"]),
#  static_library = "boost/lib/libboost_thread.a",
#  # If alwayslink is turned on,
#  # libmylib.a will be forcely linked into any binary that depends on it.
#  # alwayslink = 1,
#  visibility = ["//visibility:private"],
#  #includes = ["boost/include"],
#)

cc_library(
  name = "boost_thread",
  hdrs = glob(["boost/include/boost/thread/**/*.hpp","boost/include/boost/thread/**/*.ipp","boost/include/boost/thread.hpp"]),
  srcs = ["boost/lib/libboost_thread.a"],
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
  deps = [
#    ":boost_thread_lib",
    ":boost_enable_shared_from_this",
    ":boost_bind",
    ":boost_is_placeholder",
    ":boost_io",
    ":boost_functional",
    ":boost_atomic",
    ":boost_tuple",
    ":boost_exception_ptr",
    ":boost_next_prior",
    ":boost_iterator",
    ":boost_scoped_array",
    ":boost_optional",
    ":boost_function",
    ":boost_move",
    ":boost_date_time",
    ":boost_chrono",
  ],
)

#cc_import(
#  name = "boost_chrono_lib",
#  hdrs = glob(["boost/include/boost/chrono/**/*.hpp","boost/include/boost/chrono/**/*.ipp","boost/include/boost/chrono.hpp"]),
#  static_library = "boost/lib/libboost_chrono.a",
#  # If alwayslink is turned on,
#  # libmylib.a will be forcely linked into any binary that depends on it.
#  # alwayslink = 1,
#  visibility = ["//visibility:private"],
#)

cc_library(
  name = "boost_chrono",
  textual_hdrs = glob(["boost/include/boost/chrono/**/*.hpp","boost/include/boost/chrono/**/*.ipp","boost/include/boost/chrono.hpp"]),
  srcs = ["boost/lib/libboost_chrono.a"],
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
  deps = [
#    ":boost_chrono_lib",
    ":boost_config",
    ":boost_predef",
    ":boost_mpl",
    ":boost_ratio",
    ":boost_utility",
    ":boost_system",
    ":boost_operators",
    ":boost_integer",
    ":boost_move",
    ":boost_throw_exception",
    ":boost_date_time",
  ]
)

cc_library(
  name = "boost_ratio",
  textual_hdrs = glob(["boost/include/boost/ratio/**/*.hpp","boost/include/boost/ratio.hpp"]),
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
  deps = [
    ":boost_cstdint",
    ":boost_type_traits",
    ":boost_core",
    ":boost_integer_traits",
  ]
)

cc_library(
  name = "boost_integer_traits",
  textual_hdrs = ["boost/include/boost/integer_traits.hpp"],
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
  deps = [
    ":boost_limits",
  ]
)

cc_library(
  name = "boost_utility",
  textual_hdrs = glob(["boost/include/boost/utility/**/*.hpp","boost/include/boost/utility.hpp"]),
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "boost_integer",
  textual_hdrs = glob(["boost/include/boost/integer/**/*.hpp","boost/include/boost/integer.hpp"]),
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
  deps = [
    ":boost_assert",
    ":boost_integer_fwd",
  ],
)

cc_library(
  name = "boost_integer_fwd",
  textual_hdrs = ["boost/include/boost/integer_fwd.hpp"],
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "boost_move",
  textual_hdrs = glob(["boost/include/boost/move/**/*.hpp"]),
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "boost_io",
  textual_hdrs = glob(["boost/include/boost/io/**/*.hpp"]),
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
  deps = [
    ":boost_io_fwd",
  ],
)

cc_library(
  name = "boost_io_fwd",
  textual_hdrs = ["boost/include/boost/io_fwd.hpp"],
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "boost_functional",
  textual_hdrs = glob(["boost/include/boost/functional/**/*.hpp","boost/include/boost/functional.hpp"]),
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
  deps = [
    ":boost_container_hash",
  ],
)

cc_library(
  name = "boost_container_hash",
  textual_hdrs = glob(["boost/include/boost/container_hash/**/*.hpp","boost/include/boost/container_hash.hpp"]),
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "boost_atomic",
  textual_hdrs = glob(["boost/include/boost/atomic/**/*.hpp","boost/include/boost/atomic/**/*.ipp","boost/include/boost/atomic.hpp"]),
  srcs = ["boost/lib/libboost_atomic.a"],
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
  deps = [
  	":boost_memory_order",
  ],
)

cc_library(
  name = "boost_memory_order",
  textual_hdrs = ["boost/include/boost/memory_order.hpp"],
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "boost_tuple",
  textual_hdrs = glob(["boost/include/boost/tuple/**/*.hpp"]),
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "boost_exception_ptr",
  textual_hdrs = ["boost/include/boost/exception_ptr.hpp"],
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "boost_next_prior",
  textual_hdrs = ["boost/include/boost/next_prior.hpp"],
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "boost_iterator",
  textual_hdrs = glob(["boost/include/boost/iterator/**/*.hpp","boost/include/boost/iterator.hpp"]),
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
  deps = [
    ":boost_mpl",
  ],
)

cc_library(
  name = "boost_scoped_array",
  textual_hdrs = ["boost/include/boost/scoped_array.hpp"],
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "boost_optional",
  textual_hdrs = glob(["boost/include/boost/optional/**/*.hpp","boost/include/boost/optional.hpp"]),
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
  deps = [
    ":boost_none",
    ":boost_none_t",
  ],
)

cc_library(
  name = "boost_none",
  textual_hdrs = ["boost/include/boost/none.hpp"],
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "boost_none_t",
  textual_hdrs = ["boost/include/boost/none_t.hpp"],
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "boost_function",
  textual_hdrs = glob(["boost/include/boost/function/**/*.hpp","boost/include/boost/function.hpp"]),
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
  deps = [
    ":boost_type_index",
    ":boost_function_equal",
  ],
)

cc_library(
  name = "boost_type_index",
  textual_hdrs = glob(["boost/include/boost/type_index/**/*.hpp","boost/include/boost/type_index.hpp"]),
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "boost_function_equal",
  textual_hdrs = glob(["boost/include/boost/function_equal.hpp"]),
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "boost_filesystem",
  hdrs = glob(["boost/include/boost/filesystem/**/*.hpp","boost/include/boost/filesystem/**/*.ipp","boost/include/boost/filesystem.hpp"]),
  srcs = ["boost/lib/libboost_filesystem.a"],
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
  deps = [
    ":boost_system",
    ":boost_iterator",
    ":boost_io",
    ":boost_functional",
    ":boost_smart_ptr",
    ":boost_detail",
    ":boost_assert",
    ":boost_type_traits",
  ],
)

cc_library(
  name = "boost_dll",
  hdrs = glob(["boost/include/boost/dll/**/*.hpp","boost/include/boost/dll/**/*.ipp","boost/include/boost/dll.hpp"]),
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
  deps = [
    ":boost_swap",
    ":boost_make_shared",
    ":boost_aligned_storage",
    ":boost_noncopyable",
    ":boost_throw_exception",
    ":boost_move",
  ],
  linkopts = [
    "-lpthread",
    "-ldl",
  ],
)

cc_library(
  name = "boost_make_shared",
  textual_hdrs = ["boost/include/boost/make_shared.hpp"],
  includes = ["boost/include"],
  visibility = ["//visibility:public"],
)

#cc_import(
#  name = "boost",
#  srcs = glob(["boost_build/lib/*.a", "boost_build/lib/*.dylib", "boost_build/lib/*.so"]),
#  hdrs = glob(["boost_build/include/boost/**.hpp"]),
#)
