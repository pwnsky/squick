#! /bin/bash
#kind load docker-image pwnsky/squick:1.1

docker_image=pwnsky/squick:1.1

echo "Save image from docker"
docker save $docker_image > /tmp/squick.tar
echo "Import image"
ctr -n k8s.io image import /tmp/squick.tar
ctr -n k8s.io image list | grep squick
echo "Import ok"
