run{::options parse_block_html="true" /}
# Build instruction
## Requirements
- Bazel == 25.2
- GCC >= 7.4.0
- Docker Client >= 18.09 (API Version: 1.39)
- Docker Server >= 18.09.6 (API Version: 1.39)
- Android SDK Dev Kit (Normally installed with Android Studio)
- Android SDK 24 installed
- Android NDKr19

Test on:
- Ubuntu 18.04 (x86_64)

## Library build
<div class="panel panel-info">
**Note**
You must edit the export parameter with the location of SDK and NDK for your system
</div> 

```sh
export ANDROID_HOME=/home/mbortolon/Android/Sdk/; # if not set before
export ANDROID_NDK_HOME=/home/mbortolon/Android/Sdk/ndk-bundle; # if not set before
bazel build //src/lib:libimage_sending.so --crosstool_top=//external:android/crosstool --android_cpu=armeabi-v7a --cpu=armeabi-v7a --host_crosstool_top=@bazel_tools//tools/cpp:toolchain --linkopt="-llog"
```

## Data manager server build

```sh
bazel build //src/server:data-manager --compilation_mode=opt
```

## Data manager build docker image

```sh
bazel build //src/server:data-manager-image --compilation_mode=opt
```

## Data manager publish on gitlab

<div class="panel panel-warning">
**Note**
Your local docker installation must be configure to allow the upload of the image to the server, or the command can return an error
</div> 

```
bazel run //src/server:data-manager-publish --compilation_mode=opt
```
