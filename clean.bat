:: run the dockerfile on this directory, which will build the project
docker run -i --rm -v "%cd%":/source pikmin-nds make clean
