# OpenCV

filegroup(
    name = "src_files",
    srcs = glob(["**"]),
)

load("@rules_foreign_cc//tools/build_defs:cmake.bzl", "cmake_external")

cmake_external(
   name = "opencv",
   # Values to be passed as -Dkey=value on the CMake command line;
   # here are serving to provide some CMake script configuration options
   cache_entries = {
      "CMAKE_BUILD_TYPE": "Release",
      "BUILD_JAVA": "OFF",
      "WITH_OPENCL": "OFF",
      "BUILD_LIST": "core,imgproc,imgcodecs",
      "BUILD_PERF_TESTS": "OFF",
      "BUILD_SHARED_LIBS": "OFF",
      "BUILD_TESTS": "OFF",
      "BUILD_opencv_apps": "OFF",
      "BUILD_opencv_calib3d": "OFF",
      "BUILD_opencv_dnn": "OFF",
      "BUILD_opencv_features2d": "OFF",
      "BUILD_opencv_flann": "OFF",
      "BUILD_opencv_gapi": "OFF",
      "BUILD_opencv_highgui": "OFF",
      "BUILD_opencv_java_bindings_generator": "OFF",
      "BUILD_opencv_js": "OFF",
      "BUILD_opencv_ml": "OFF",
      "BUILD_opencv_objdetect": "OFF",
      "BUILD_opencv_photo": "OFF",
      "BUILD_opencv_python3": "OFF",
      "BUILD_opencv_python_bindings_generator": "OFF",
      "BUILD_opencv_stitching": "OFF",
      "BUILD_opencv_ts": "OFF",
      "BUILD_opencv_video": "OFF",
      "BUILD_opencv_videoio": "OFF",
      "cmake_CONFIGURATION_TYPES": "OFF",
      "OPENCV_FORCE_3RDPARTY_BUILD": "ON",
      "WITH_ADE": "OFF",
      "WITH_IPP": "OFF",
      "WITH_ITT": "OFF",
      "WITH_PROTOBUF": "OFF",
      "BUILD_ANDROID_EXAMPLES": "OFF",
   },
   lib_source = ":src_files",
   out_include_dir = "include/opencv4",
   # We are selecting the resulting static library to be passed in C/C++ provider
   # as the result of the build;
   # However, the cmake_external dependants could use other artefacts provided by the build,
   # according to their CMake script
   static_libraries = [
        "libopencv_imgcodecs.a",
        "libopencv_imgproc.a",
        "libopencv_core.a",
        "opencv4/3rdparty/liblibjasper.a",
#        "opencv4/3rdparty/libittnotify.a",
        "opencv4/3rdparty/liblibwebp.a",
        "opencv4/3rdparty/liblibjpeg-turbo.a",
        "opencv4/3rdparty/libquirc.a",
        "opencv4/3rdparty/liblibtiff.a",
        "opencv4/3rdparty/liblibpng.a",
        "opencv4/3rdparty/libIlmImf.a",
        "opencv4/3rdparty/libzlib.a",
#        "opencv4/3rdparty/liblibprotobuf.a",
   ],
#   shared_libraries = [
#        "libopencv_core.so.4.1.0",
#        "libopencv_imgproc.so.4.1.0",
#        "libopencv_imgcodecs.so.4.1.0",
#   ],
   visibility = ["//visibility:public"],
)
