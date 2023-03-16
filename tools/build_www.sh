#! /bin/bash
# build script for squick www
# author: i0gan
# date: 2022-11-19

ProjectPath=..

build_www_server() {
	cd ${ProjectPath}/www/server
	bash ./build.sh
	cd ${ProjectPath}
}

build_admin() {
	cd ${ProjectPath}/www/admin
	bash ./build.sh
	cd ${ProjectPath}
}

build_website() {
	cd ${ProjectPath}/www/website
	#bash ./build.sh
	cd ${ProjectPath}
}

# build
time build_www_server $@
time build_admin
time build_website
