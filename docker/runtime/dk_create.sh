#! /bin/bash

docker build -t i0gan/squick-runtime:1.0 .

cd ../../
project_path=`pwd`
#cd ./docker/dev

docker stop squick-runtime
docker rm squick-runtime 
docker run -d --name=squick-runtime -v $project_path:/mnt --net=host i0gan/squick-runtime:1.0 /mnt/docker/runtime/service.sh
