#! /bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2023-03-16
# Github: https://github.com/pwnsky/squick
# Description: Create a mysql database

docker pull mysql:8.0
project_path=../../
mysql_data_path=${project_path}/data/mysql/data
mysql_log_path=${project_path}/data/mysql/log
mysql_files_path=${project_path}/data/mysql/files

docker run  --restart=always  --name squick_mysql  -v `pwd`/conf:/etc/mysql  -v $mysql_data_path:/var/lib/mysql  -v $mysql_log_path:/var/log  -v $mysql_files_path:/var/lib/mysql-files -p 10086:3306  -e MYSQL_ROOT_PASSWORD='pwnsky_squick' -d mysql:8.0

# ref: https://blog.csdn.net/piaomiao_/article/details/119241127

