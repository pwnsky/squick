#! /bin/bash
docker build -t squick_dev .

cd ../../
project_path=`pwd`
#cd ./docker/dev

docker stop squick
docker rm squick
docker run -d --name=squick -v $project_path:/mnt --net=host squick_dev:latest /mnt/docker/dev/service.sh
