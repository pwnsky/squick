#! /bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2023-03-10
# Github: https://github.com/pwnsky/squick
# Description: Generate Linux or Macos deploy files
cd $(dirname $0)
source common.sh
echo "Generating linux or macos deploy files"
bash init_runtime_dll.sh

cd ..
rm -rf deploy
mkdir deploy
mkdir deploy/src

cp -r bin deploy
cp -r config deploy
cp -r script deploy
cp -r src/lua deploy/src

# copy res
mkdir -p deploy/res
cp -r res/XlsxXML deploy/res/XlsxXML
cp -r res/Proto deploy/res/Proto

mkdir -p deploy/tools
cp -r tools/flamegraph deploy/tools
cp -r tools/binary_strip.sh deploy/tools

# copy lib
echo "Deploy for $LinuxDistro"
if [ $LinuxDistro == "Debian" ] || [ $LinuxDistro == "Ubuntu" ] || [ $LinuxDistro == "Raspbian" ]; then
    cp /lib/x86_64-linux-gnu/libssl.so.1.1    deploy/bin
    cp /lib/x86_64-linux-gnu/libcrypto.so.1.1 deploy/bin
    cp /lib/x86_64-linux-gnu/libssl.so    deploy/bin
    cp /lib/x86_64-linux-gnu/libcrypto.so deploy/bin
elif [ $LinuxDistro == "Arch" ]; then
    echo "Arch"
fi

# create dir
cd deploy
mkdir -p data/logs
mkdir -p data/crashs

echo "Generated all files!"
