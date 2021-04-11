sudo chown $USER /dev/ttyUSB0;
docker run -it --rm --privileged --name lorsi_pdm --user $(id -u):$(id -g) --workdir /app -v "$(pwd)":/app -v  /dev:/dev lorsi/pdm /bin/bash ;
