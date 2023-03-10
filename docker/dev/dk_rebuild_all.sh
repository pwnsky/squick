#! /bin/bash
echo "dev rebuild"
docker start squick
docker exec -d squick /mnt/tools/stopall.sh
docker exec -t squick /bin/bash -c "cd /mnt/tools && bash ./build_all.sh"
docker exec -d squick /mnt/tools/startall.sh
