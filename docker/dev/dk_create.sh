#! /bin/bash
docker build -t i0gan/squick-dev:1.0 .

cd ../../
project_path=`pwd`
#cd ./docker/dev

docker stop squick-dev
docker rm squick-dev
docker run -d --name=squick -v $project_path:/mnt --net=host i0gan/squick-dev:1.0 /mnt/docker/dev/service.sh
