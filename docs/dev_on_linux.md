## Linux或MacOS上开发和编译

Linux k8s is used for deployment. The code needs to be compiled before deployment. The following two docker containers are provided for developers to use.

```
i0gan/squick-dev     # The squick compiles and packages the container, version 1.0
i0gan/squick-runtime # The squick program runs the container, version 1.0
```

If you need to write automated packaging and deployment, developers need to write automated scripts themselves.


你可以直接任意选择你自己喜欢的开发方式，可以采用CMake生成 Qt工程，或直接采用Ridder打开CMake工程，以下提供了两种编译方式。

[1. 直接编译](Linux直接编译)

[2. Linux容器编译 ](Linux容器编译 ) (推荐)

默认编译的是debug版本，如果想编译为release版本，请打开{project_path}/build.sh，在第8行的Version改为release即可。在编译的时候，会在项目根目录下创建一个cache文件夹来存储编译时生产的临时中间文件。



### 直接编译

采用物理机编译是为了开发以及测试更加方便。但可能需要你们自己手动配置各种搭建环境时出现的问题，可能比较老的Linux会出现编译不了。

#### 编译squick

克隆代码

```
git clone https://github.com/pwnsky/Squick.git
cd Squick/tools
bash install_dev_env.sh
```

install_dev_env.sh脚本会自动下载编译工具，并且编译，如果出现不能编译，可能是缺少是依赖，linux依赖包如下：

```
git cmake unzip automake make g++ libtool libreadline6-dev libncurses5-dev pkg-config libssl-dev
nodejs npm libjsoncpp-dev uuid-dev zlib1g-dev
```

请采用手动进行安装以上工具包。

进入到cd {project_path}/tools

**编译第三方库**

```sh
bash build_third_party.sh
```

**编译sqkctl**

```
bash build_sqkctl.sh
```

**生成配置文件**

```
bash generate_config.sh
```

**编译squick** 

```
bash build_squick.sh
```



### 编译后的文件

编译完成后，在`{project_path}/deploy/bin` 下会出现编译好的二进制文件。Linux如下

```
deploy/bin/
├── core.so
...
├── plugin
│   ├── core
│   │   ├── actor.so
...
│   └── node
...
└── squick
```

MacOS如下
```
deploy/bin/
├── core.dylib
...
├── plugin
│   ├── core
│   │   ├── actor.dylib
│   └── node
...
└── squick

```



## Start

在 {project_path}/deploy目录下，单个进程启动所有服务器命令如下：

linux执行

```bash
bash ./single_start.sh
```

window执行

```
single_start.bat
```

看到了启动各个服务器的界面，说明你已经完成编译以及启动了。

除此之外，还支持命令启动独立的服务器，比如：

```
squick plugin=master.xml server=master id=3
```
