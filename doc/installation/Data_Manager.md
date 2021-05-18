# data-manager installation

## Prebuilt Docker Image (recommended way)

This modality requires Docker on our OS. 
To install it follow the [Docker documentation](https://docs.docker.com/engine/install/). 
This modality was tested both on macOS Catalina and Ubuntu 20.04 Focal.

1) Download the latest ZIP file from the [release page](https://github.com/fabiopoiesi/4dm/releases).
2) Extract its content.
3) Install the data-manager image using the command:

```bash
docker load < [absolute path to extracted folder]/data-manager.tar
```

4) Start the process using the following command. Note: replace bind directory with the directory where you want to save the capture sessions.

```bash
docker run -d -v [bind directory]:/tmp/uploadedImages/ -p 5959:5959 -p 5960:5960 data-manager:v0.8
```

5) Check that it is all working correctly with the command:
6) 
```bash
docker ps -a
```

Here below what you should see as a result of the previous command:

```
CONTAINER ID   IMAGE               COMMAND           CREATED          STATUS          PORTS                                                           NAMES
faa8b166760e   data-manager:v0.8   "/data-manager"   40 seconds ago   Up 39 seconds   0.0.0.0:5959-5960->5959-5960/tcp, :::5959-5960->5959-5960/tcp   exciting_matsumoto
```

## Build Docker image from source

To executing this procedure it is necessary to install Bazel 4.0.0.
It might work with older versions, but we do not guarantee this.
To install it please follow [Bazel website](https://docs.bazel.build/versions/4.0.0/install.html). 
Note: the data-manager does not work under Windows.

1) Download the source code of the repository using:

```bash
git clone git@github.com:fabiopoiesi/4dm.git
```

2) Access the subdirectory that contains the code of the data manager and build the executable. 
This procedure requires the download of several dependencies, it may require some time:

```bash
cd server
bazel build //src/server:data-manager --compilation_mode=opt
```

While the use of Bazel should guarantee a repetable build, our experience shows that this is not always the case, in the case of issues please post it in this github.

3) If the compilation succeed, Bazel produces a binary file inside the ```bazel-bin``` directory, that you can execute with the command:

```bash
./bazel-bin/src/server/data-manager
```

The results will be saved inside /tmp/uploadedImages.
