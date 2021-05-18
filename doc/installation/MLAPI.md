# mlapi-server installation

This modality requires Docker on your OS. 
To install it follow the [Docker documentation](https://docs.docker.com/engine/install/). 
This modality was tested both on macOS Catalina and Ubuntu 20.04 Focal.

## Prebuilt Docker Image (recommended way)

1) Download the latest ZIP file from the [release page](https://github.com/fabiopoiesi/4dm/releases). 
2) Extract its content.
3) Install the mlapi-server image using the command:

```bash
docker load < [absolute path to extracted folder]/mlapi-docker.tar
```

4) Start the process using the command:

```bash
docker run -d -p 8889:8889 --network="host" mlapi-docker:v0.8
```

It is necessary to set the network mode to ```host``` because the default mode hides the container behind a NAT and this makes impossible in the current state to correctly the slaves with the master, currently the binding is based on the IP address.

5) Check that it is all working correctly with the command:

```bash
docker ps -a
```

Here below what you should see as a result of the previous command:

```
CONTAINER ID   IMAGE               COMMAND          CREATED         STATUS         PORTS     NAMES
1f7db64c6208   mlapi-docker:v0.8   "./mlapiRelay"   7 seconds ago   Up 6 seconds             fervent_banzai
```

## Build Docker image from source

Download the source code of the repository using:

```bash
git clone git@github.com:fabiopoiesi/4dm.git
```

Access the subdirectory that contains the dockerize version of mlapi-server and build the image:

```bash
cd mlapi-server
docker sudo build -t mlapi-docker:v0.8 .
```
