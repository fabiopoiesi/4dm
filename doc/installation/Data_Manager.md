# Data Manager Installation

## As a prebuilt Docker Image (recommended way)

Running in this mode require that Docker is installed, to install it please follow the [Docker documentation](https://docs.docker.com/engine/install/). This modality was tested both on macOS Catalina and Ubuntu 20.04 Focal.

First it is necessary to download from the [release page](https://github.com/fabiopoiesi/4dm/releases) the latest ZIP containing the Docker images and the APK. After extracting it, you can install the MLAPI image using the command:

```bash
docker load < [absolute path to extracted folder]/data-manager.tar
```

Then you can start it using the command (replace bind directory with the directory where you want to save the acquired sessions):

```bash
docker run -d -v [bind directory]:/tmp/uploadedImages/ -p 5959:5959 -p 5960:5960 data-manager:v0.8
```

You can check that everything work correctly with the command:
```bash
docker ps -a
```

The result should be similar to:
```
CONTAINER ID   IMAGE               COMMAND           CREATED          STATUS          PORTS                                                           NAMES
faa8b166760e   data-manager:v0.8   "/data-manager"   40 seconds ago   Up 39 seconds   0.0.0.0:5959-5960->5959-5960/tcp, :::5959-5960->5959-5960/tcp   exciting_matsumoto
```

## Build data manager from source

For executing this procedure is necessary to install Bazel 4.0.0, it is possible that it work also with never version but we do not guaranted this. To install it please follow the procedure specify on t [Bazel website](https://docs.bazel.build/versions/4.0.0/install.html). The Data Manager do not work in Windows.

If not already done, download the source code of the repository using:

```bash
git clone git@github.com:fabiopoiesi/4dm.git
```

Access the subdirectory that contain the code of the data manager and build the executable, the procedure need to download all the dependencies, so this may require some time:
```bash
cd server
bazel build //src/server:data-manager --compilation_mode=opt
```

While the use of Bazel should be guaranted a repetable build, our experience show that this is not always the case, in case of troubles fell free to open a issue or contact us and we would happy to help.

At the end, if the compilation success, Bazel produce the binary inside the ```bazel-bin``` directory, that you can execute with the command:
```bash
./bazel-bin/src/server/data-manager
```

The results will be saved inside /tmp/uploadedImages