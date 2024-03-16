### Linux容器编译

该方式适用于打包，并不适合开发，开发在Windows平台开发。

**推荐**

#### 1. 创建容器

这种是方式编译，懒得自己搭建开发环境，也是为了方便开发，让编译文件与开发文件直接映射，采用的是ubuntu:20.04环境来进行编译的。

```sh
cd {project_path}/docker/dev/
bash dk_create.sh
```

如果执行失败，可以拉取已搭建好的容器

```
docker pull i0gan/squick-dev:1.0
```

再次运行

```
docker run -d --name=squick-dev -v $project_path:/mnt --net=host i0gan/squick-dev:1.0 /mnt/docker/dev/service.sh
```

`$project_path`改为squick项目所在目录，可以参考`dk_create.sh`脚本。

#### 2. 进入容器执行命令进行编译

```sh
cd /mnt/tools

# 编译squick 第三方库
bash build_third_party.sh

# 编译sqkctl工具
bash build_sqkctl.sh

# 编译squick代码
bash build_squick.sh
```

#### 测试打包运行

```sh
bash generate_deploy.sh
cd ../deploy
./single_start.sh
```