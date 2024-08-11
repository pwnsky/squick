#! /bin/bash

cd $(dirname $0)
source ../tools/common.sh

build_type='docker'
is_build_third_party=0
is_build_sqkctl=0
src_image_tag=pwnsky/squick_src:1.2
bin_image_tag=pwnsky/squick:1.2
build_container=squick_src_build

cd $project_path

echo "Build src image"
docker build . -t $src_image_tag -f ./docker/src
check_err

echo "Export binary files"
docker run -d --name $build_container $src_image_tag
check_err
rm -rf ./cache/docker_deploy
docker cp $build_container:/squick/deploy ./cache/docker_deploy
check_err

echo "Clean src build env"
docker stop $build_container
check_err
docker rm $build_container
check_err

echo "Build release image"
docker build . -t $bin_image_tag -f ./docker/release
check_err

echo "Build image ok"
print_ok
