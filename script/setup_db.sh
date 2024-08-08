#! /bin/bash

echo "set up mysql"
docker run -d --restart always --name squick_db_mysql_1 -p 10400:33060 -e MYSQL_ROOT_PASSWORD=pwnsky_squick  mysql:8.0

echo "set up mongo"
docker run -d --restart always --name squick_db_mongo_1 -p 10410:27017 -e MONGO_INITDB_ROOT_USERNAME=admin -e MONGO_INITDB_ROOT_PASSWORD=pwnsky_squick mongo:6.0.5 mongod --auth

echo "set up redis"
docker run -d --restart always --name squick_db_redis_1 -p 10420:6379 redis:7.0 --requirepass pwnsky_squick

echo "set up clickhouse"
docker run -d --restart always --name squick_db_clickhouse_1 -p 10430:8123 -p 10431:9000 -p 10432:9009 clickhouse/clickhouse-server:23.1.3.5-alpine

echo "please set the db password in clickhouse"
echo "Find <password></password>"
echo "Add the password(pwnsky_squick) in the middle"
sleep 3
docker exec -it squick_db_clickhouse_1 vi /etc/clickhouse-server/users.xml
