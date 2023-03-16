#! /bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2022-12-03
# Github: https://github.com/i0gan/Squick
# Description: Build tutorial code for beginner

source source/build.sh

build_tutorial() {
	cd ${project_path}
	mkdir -p "${build_path}/tutorial"
	cd "${build_path}/tutorial"
	cmake ${project_path}/src/tutorial -G "CodeBlocks - Unix Makefiles" -DCMAKE_BUILD_TYPE=$build_version
	if [ $# -gt 0 ]; then
		# Compile all
		echo "Compile $@"
		make $@ -j $(nproc)
	else
		echo "Compile all"
		make -j $(nproc)
	fi
}


# build
time build_tutorial $@