#!/bin/bash
# Author: i0gan
# Email: l418894113@gmail.com
# Date: 2022-10-01
# Github: https://github.com/i0gan/Squick
# Description: Bash script source file

build_type="Release"
#build_version="Debug"
build_mode="dev"
project_path=`pwd`/..
build_path="$project_path/cache"
sys=`uname -s`
make_threads=$(nproc)


function log_error()
{
     echo -e "\e[1;41m Error: $1 \e[0m"
     sleep 1
}

function log_info()
{
    echo -e "\e[1;42m $1\e[0m"
    sleep 1
}

function log_debug()
{
    echo -e "\e[5;45m $1 \e[0m"
    sleep 1
}

function check_err()
{
    errno=$?
    if [[ $errno != 0 ]];then
        log_error "Has terminated process, The error number: $errno"
        exit
    else
        log_info "No error"
    fi
}

