#!/bin/sh

# build the docker image if necessary
docker build \
  --build-arg external_username=$(id --user --name) \
  --build-arg external_uid=$(id --user) \
  --build-arg external_gid=$(id --group) \
  -t pikmin-nds .

# run the dockerfile on this directory, which will build the project
docker run -i -t --rm -v $(pwd):/source pikmin-nds "$@"
