#! /bin/bash

array=("web" "proxy1" "player" "world" "backstage" "master" "db_proxy" "db_mysql" "db_redis" "db_mongo" "db_clickhouse")

for i in "${array[@]}"; do
  container_name="docker-$i-1"
  echo "del $container_name"
  docker kill $container_name
  docker rm $container_name
done
