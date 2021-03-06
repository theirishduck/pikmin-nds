:: build the docker image if necessary
docker build --build-arg external_username=docker --build-arg external_uid=1000 --build-arg external_gid=1000 -t pikmin-nds %cd%

:: run the dockerfile on this directory, which will build the project
docker run -i -t --rm -v "%cd%":/source pikmin-nds %*
