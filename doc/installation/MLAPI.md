# mlapi-server installation

## Prebuilt Docker Image (recommended way)

This modality requires Docker on your OS. 
To install it follow the [Docker documentation](https://docs.docker.com/engine/install/). 
This modality was tested both on macOS Catalina and Ubuntu 20.04 Focal.

1) Download the latest ZIP from the [release page](https://github.com/fabiopoiesi/4dm/releases). 
After extracting its content, you can install the mlapi-server image using the command:

```bash
docker load < [absolute path to extracted folder]/mlapi-docker.tar
```

Then you can start the process using the command: 

```bash
docker run -d -p 8889:8889 --network="host" mlapi-docker:v0.8
```

It is necessary to set the network mode to ```host``` because the default mode hides the container behind a NAT and this makes impossible in the current state to correctly the slaves with the master, currently the binding is based on the IP address.

You can check that everything works correctly with the command:

```bash
docker ps -a
```

The result should be similar to:
```
CONTAINER ID   IMAGE               COMMAND          CREATED         STATUS         PORTS     NAMES
1f7db64c6208   mlapi-docker:v0.8   "./mlapiRelay"   7 seconds ago   Up 6 seconds             fervent_banzai
```

## Build docker image from source

Running in this mode require that Docker is installed, to install it please follow the [Docker documentation](https://docs.docker.com/engine/install/). This modality was tested both on macOS Catalina and Ubuntu 20.04 Focal.

If not already done, download the source code of the repository using:

```bash
git clone git@github.com:fabiopoiesi/4dm.git
```

Access the subdirectory that contain the dockerize version of MLAPI and build the image:
```bash
cd mlapi-server
docker sudo build -t mlapi-docker:v0.8 .
```
