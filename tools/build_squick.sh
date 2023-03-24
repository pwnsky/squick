#! /bin/bash
# Author: i0gan
# Email : i0gan@pwnsky.com
# Date  : 2022-09-25
# Github: https://github.com/pwnsky/squick
# Description: Build squick application and core plugins

source source/build.sh

# 将Proto文件生成代码
cd ${project_path}/tools
bash ./proto2code.sh

# 生成配置文件
cd ${project_path}/tools/
bash ./generate_config.sh

# 初始化第三方.so
cd ${project_path}/tools/
bash ./init_runtime_dll.sh

build_squick() {
	cd ${project_path}
	mkdir -p "${build_path}"
	cd "${build_path}"
	cmake ${project_path}/src  -DBUILD_VERSION=$build_version -DMODE=$build_mode
	if [ $# -gt 0 ]; then
		echo "Compile $@"
		make $@ -j $(nproc)
	else
		echo "Compile all"
		make -j $(nproc)
	fi
}

# build
time build_squick $@

