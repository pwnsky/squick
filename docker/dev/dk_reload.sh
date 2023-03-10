#! /bin/bash
echo "dev rebuild"
docker start squick
docker exec -t squick /bin/bash -c "cd /mnt/tools && bash stopall.sh"
docker exec -t squick /bin/bash -c "cd /mnt/tools && bash build_squick.sh && bash ./startall.sh"
docker exec -d squick /bin/bash -c "cd /mnt/tools && bash startall.sh"
