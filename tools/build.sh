#! /bin/bash
# Author: i0gan
# Email : i0gan@pwnsky.com
# Date  : 2022-09-25
# Github: https://github.com/pwnsky/squick
# Description: Build squick application and all plugins

cd $(dirname $0)
source common.sh

function init_env()
{
    # 将Proto文件生成代码
    log_info "Generate protocol c++ files from proto files"
    cd ${project_path}/res
    time bash ./GenProtoCode.sh server
    check_err
    
    # 生成配置文件
    log_info "Generate configure files"
    cd ${project_path}/res
    time bash ./ExportTable.sh server
    check_err
    
    # 初始化第三方.so
    log_info "Init runtime env"
    cd ${project_path}/tools/
    time bash ./init_runtime_dll.sh
    check_err
}

log_info "Start to build squick"
build_squick() {
	cd ${project_path}
	mkdir -p "${build_path}"
	if [ $# -gt 0 ]; then
		echo "Compile $@"
	    cd "${build_path}"
	    cmake ${project_path}  -DBUILD_VERSION=$build_version -DMODE=$build_mode
		make $@ -j $(nproc)
	else
		echo "Compile all"
        init_env
	    cd "${build_path}"
	    cmake ${project_path}  -DBUILD_VERSION=$build_version -DMODE=$build_mode
		make -j $(nproc)
	fi
}

# build
time build_squick $@
check_err
log_info "All builded"
print_ok
