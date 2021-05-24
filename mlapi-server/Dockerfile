FROM microsoft/dotnet:2.2-sdk as builder

ARG DOWNLOAD_URL=https://github.com/MidLevel/MLAPI.Relay/archive/master.zip

COPY additionalLib /tmp/additionalLib
COPY MLAPI-Relay /tmp/MLAPI-Relay

RUN if [ ! -f /tmp/MLAPI-Relay/MLAPI.Relay-master/MLAPI.Relay.sln ]; then \
        apt-get update && apt-get install -y --no-install-recommends unzip && rm -rf /var/lib/apt/lists/*; \
        wget -q ${DOWNLOAD_URL} -O /tmp/MLAPI-Relay.zip; \
        mkdir /tmp/MLAPI-Relay; \
        unzip /tmp/MLAPI-Relay.zip -d /tmp/MLAPI-Relay; \
        rm -f /tmp/MLAPI-Relay.zip; \
    fi; \
    cp -rnf /tmp/additionalLib/. /tmp/MLAPI-Relay/MLAPI.Relay-master/MLAPI.Relay/Libs; \
    rm -rf /tmp/additionalLib; \
    cd /tmp/MLAPI-Relay/MLAPI.Relay-master; \
    dotnet publish -c Release; \
    cp -rnf /tmp/MLAPI-Relay/MLAPI.Relay-master/MLAPI.Relay/Libs/. /tmp/MLAPI-Relay/MLAPI.Relay-master/MLAPI.Relay/bin/Release/netcoreapp2.0/publish;

COPY config.json /tmp/MLAPI-Relay/MLAPI.Relay-master/MLAPI.Relay/bin/Release/netcoreapp2.0/publish

FROM microsoft/dotnet:2.2-runtime

COPY --from=builder /tmp/MLAPI-Relay/MLAPI.Relay-master/MLAPI.Relay/bin/Release/netcoreapp2.0/publish /usr/bin/MLAPI-Relay
COPY mlapiRelay /mlapiRelay

EXPOSE 8888

CMD [ "./mlapiRelay" ]