# Data Manager Installation

## As a prebuilt Docker Image (recommended way)

Running in this mode require that Docker is installed, to install it please follow the [Docker documentation](https://docs.docker.com/engine/install/). This modality was tested both on macOS Catalina and Ubuntu 20.04 Focal.

First it is necessary to download from the [release page](https://github.com/fabiopoiesi/4dm/releases) the latest ZIP containing the Docker images and the APK. After extracting it, you can install the MLAPI image using the command:

```bash
docker load < [absolute path to extracted folder]/data-manager.tar
```

Then you can start it using the command: 

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

## As binary

In progress