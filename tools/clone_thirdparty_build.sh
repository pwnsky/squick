#! /bin/bash
git clone https://github.com/pwnsky/squick-thirdparty-build.git
rm -rf ../third_party/build
cp -r squick-thirdparty-build/Linux/ubuntu20/build ../third_party/build
rm -rf squick-thirdparty-build
