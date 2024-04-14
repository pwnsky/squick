#! /bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2022-10-01
# Github: https://github.com/pwnsky/squick
# Description: Install development environment on linux


source common.sh

# for macos
if [ $sys == "Darwin" ];then
    echo "MacOS env"
    brew install cmake
    brew install openssl
    exit
else
    echo "Linux env"
fi


echo "Current LinuxDistro $LinuxDistro"

if [ $LinuxDistro == "Debian" ] || [ $LinuxDistro == "Ubuntu" ] || [ $LinuxDistro == "Raspbian" ]; then
    $sudo apt-get update
    $sudo apt-get -y git cmake unzip automake make g++ libtool pkg-config
    $sudo apt-get -y install libreadline-dev libssl-dev libncurses5-dev
    
elif [ $LinuxDistro == "CentOS" ] || [ $LinuxDistro == "RHEL" ] || [ $LinuxDistro == "Fedora" ]  || [ $LinuxDistro == "Aliyun" ]; then
    $sudo yum -y install cmake unzip automake make
    $sudo yum -y install gcc-c++
    $sudo yum -y install libtool
    $sudo yum -y install readline-devel
    $sudo yum -y install ncurses-devel
    $sudo yum -y install pkg-config
else # arch
    yes | $sudo pacman -Sy cmake unzip automake make
    yes | $sudo pacman -Sy gcc
    yes | $sudo pacman -S libtool
    #yes | $sudo pacman -S libreadline
    #yes | $sudo pacman -S libncurses
    yes | $sudo pacman -S pkg-config
    yes | $sudo pacman -S python
    yes | $sudo pacman -S openssl
fi
