#!/bin/sh

# build the docker file if it isn't already here
docker build --quiet=true -t zeta0134/pikmin-nds ./docker

# run the dockerfile on this directory, which will build the project
docker run -i --rm -v $(pwd):/source zeta0134/pikmin-nds