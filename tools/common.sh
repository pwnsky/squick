#!/bin/bash
# Author: i0gan
# Email: l418894113@gmail.com
# Date: 2022-10-01
# Github: https://github.com/pwnsky/squick
# Description: Bash script source file

# Build version
if [ -z $SQUICK_BUILD_VERSION ];then
        SQUICK_BUILD_VERSION="Debug"
fi
echo "build version: $SQUICK_BUILD_VERSION"

# Build mode
if [ -z $SQUICK_BUILD_MODE ];then
        SQUICK_BUILD_MODE="dev"
fi
echo "build mode: $SQUICK_BUILD_MODE"

build_version=$SQUICK_BUILD_VERSION
#build_version="Release"

build_mode=$SQUICK_BUILD_MODE
project_path=`pwd`/..
build_path="$project_path/cache"
sys=`uname -s`
make_threads=$(nproc)


# Os
if grep -Eqi "CentOS" /etc/issue || grep -Eq "CentOS" /etc/*-release; then
    LinuxDistro='CentOS'
elif grep -Eqi "Red Hat Enterprise Linux Server" /etc/issue || grep -Eq "Red Hat Enterprise Linux Server" /etc/*-release; then
    LinuxDistro='RHEL'
elif grep -Eqi "Aliyun" /etc/issue || grep -Eq "Aliyun" /etc/*-release; then
    LinuxDistro='Aliyun'
elif grep -Eqi "Fedora" /etc/issue || grep -Eq "Fedora" /etc/*-release; then
    LinuxDistro='Fedora'
elif grep -Eqi "Debian" /etc/issue || grep -Eq "Debian" /etc/*-release; then
    LinuxDistro='Debian'
elif grep -Eqi "Ubuntu" /etc/issue || grep -Eq "Ubuntu" /etc/*-release; then
    LinuxDistro='Ubuntu'
elif grep -Eqi "Raspbian" /etc/issue || grep -Eq "Raspbian" /etc/*-release; then
    LinuxDistro='Raspbian'
elif grep -Eqi "Arch" /etc/issue || grep -Eq "Arch" /etc/*-release; then
    LinuxDistro='Arch'
else
    LinuxDistro='unknow'
fi


# for linux
who=`whoami`
if [[ $who == "root" ]];then
    sudo=""
else
    sudo="sudo"
fi

function log_error()
{
     echo -e "\e[1;41m ERROR: $1 \e[0m"
     sleep 1
}

function log_info()
{
    echo -e "\e[1;42m INFO: $1\e[0m"
}

function log_debug()
{
    echo -e "\e[5;45m INFO: $1 \e[0m"
}

function check_err()
{
    errno=$?
    if [[ $errno != 0 ]];then
        log_error "Has terminated process, The error number: $errno"
        exit $errno
    fi
}

function print_ok()
{
	text="Life is a journey, not a destination. - Ralph Waldo Emerson"
	echo -e "\e[5;32m$text\e[0m"

}
