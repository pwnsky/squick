## 搭建数据库

数据库涉及四种类型的数据库，mysql、mongodb、redis、Clickhouse 这三种数据库职责也不同。搭建数据库，推荐采用docker来创建数据库实例，如果没有安装docker，请通过你分支下的包管理命令进行安装。

### 搭建Mysql玩家账号数据库

负责登录以及玩家的账号数据，本次数据库采用最新版本的mysql8。

```
docker run -d --restart always --name squick_db_mysql_1 -p 10400:33060 -e MYSQL_ROOT_PASSWORD=pwnsky_squick  mysql:8.0
```

pwnsky_squick 是密码, 导入基本sql，在 {project_path}/resource/mysql 目录下

```
docker exec -it squick_db_mysql_1 bash
mysql -uroot -ppwnsky_squick
# 复制粘贴sql代码执行即可
```


### 搭建Mongo游戏数据库

存储玩家游戏里的数据。

拉取mongo镜像并创建运行mongo容器

```
docker run -d --restart always --name squick_db_mongo_1 -p 10410:27017 -e MONGO_INITDB_ROOT_USERNAME=admin -e MONGO_INITDB_ROOT_PASSWORD=pwnsky_squick mongo:6.0.5 mongod --auth
```



### 搭建Redis缓存数据库

负责缓存Squick服务器之间的数据

```
docker run -d --restart always --name squick_db_redis_1 -p 10420:6379 redis:7.0 --requirepass pwnsky_squick
```



### 搭建Clickhouse日志数据库

负责记录服务器日志、游戏日志、埋点数据等。

```
docker run -d --restart always --name squick_db_clickhouse_1 -p 10430:8123 -p 10431:9000 -p 10432:9009 clickhouse/clickhouse-server:23.1.3.5-alpine
```

创建之后进入容器

```
docker exec -it squick_db_clickhouse_1 bash
vi /etc/clickhouse-server/users.xml

# 找到 <password></password>
# 在中间增加 pwnsky_squick
```



## 修改配置文件

### 修改数据库IP

采用Office软件打开{project_path}/resource/excel/DB.xlsx，修改里面的IP为你搭建的数据库ip，默认为127.0.0.1。

### 修改服务器公网IP,

打开{project_path}/resource/excel/Server.xlsx, 修改PublicIP为服务器的公网IP。

### 生成配置

修改完毕之后，需要重新生产配置文件，需执行一个脚本进行生成。linux执行如下：

```
cd {project_path}/tools
bash generate_config.sh
```

windows执行

```
cd {project_path}/tools
generate_config.bat
```
