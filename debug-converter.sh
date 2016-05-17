#!/bin/sh

# build the docker image if necessary
docker build -t zeta0134/pikmin-nds ./docker

# run the dockerfile on this directory, which will build the project
docker run -it --rm -v $(pwd):/source -v $(pwd)/../dsgx-converter:/opt/dsgx-converter zeta0134/pikmin-nds bash
