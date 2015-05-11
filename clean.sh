#!/bin/sh

# run the dockerfile on this directory, which will build the project
docker run -i --rm -v $(pwd):/source zeta0134/pikmin-nds make clean