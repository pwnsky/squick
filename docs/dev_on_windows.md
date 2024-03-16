

# Develop on Windows

It is recommended for development to use Virsual Studio 2022 above editor and Vscode under the Windows platform to write code. This tool is also used for code debugging. Under Windows, the third-party library only has a Debug version.

## Use Virtual Stuio for development

Before development, you need to download the corresponding toolkit, as follows:

### Download the base package and install it

**cmake**

Download link: https://cmake.org/download/

When clicking Install, remember to select the Add environment variables option.

**python3**

Download link: https://www.python.org/downloads/windows/

When clicking Install, remember to select the Add environment variables option.

## 1. Third-party library

Copy the build under https://github.com/pwnsky/SquickThirdPartyBuild/tree/main/Windows/ to the {project_path}/third_party directory, so that you can't compile the third-party source code by yourself.


## 2 sqkctl tool compilation

First, when compiling for the first time, compile the sqkctl tool first, enter the tool directory ({project_path}/tools), and click build_sqkctl.bat to compile. After compilation, click generate_config.bat to generate the corresponding configuration file and code file. Run register_env.bat with administrator privileges to register squick's environment variables (note: the rest of the scripts are run with normal privileges, only register_env.bat requires administrator privileges).


## 3. Main project compilation

After compiling or processing third-party dependent libraries, you can directly compile the main project source code.

Then click generate_vs_project.bat to generate the vs project project under {project_path}/cache, open {project_path}/cache/Project.sln to compile all. Set the squick_exe project as the startup item and modify the debugging working directory to {project_path}/bin. You can also set corresponding parameters to debug different servers. Then you can use VS to start and debug all servers.

After compilation is completed, the file directory is roughly as follows:

```
bin
├── core.dll
...
├── plugin
│   ├── core
│   │   ├── actor.dll
...
│   └── node
...
├── squick.exe
..
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



## Pack

Packaging can be divided into Windows packaging and Linux packaging

### Windows

点击generate_deploy.bat

### Linux

```
bash generate_deploy.sh
```


After running, all packaged files will be saved under {project_path}/deploy directory.