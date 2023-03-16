#! /bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2022-11-27
# Github: https://github.com/i0gan/Squick
# Description: build script for squick

bash ./build_third_party.sh
bash ./build_tools.sh
bash ./proto2code.sh
bash ./generate_config.sh
bash ./build_squick.sh
#bash ./build_test.sh
#bash ./build_tutorial.sh

#cd ../src/www/admin/
#rm -rf node_modules
#npm config set registry https://registry.npm.taobao.org
#git config --global url."https://".insteadOf git:// && npm install
#cd ../../../tools
#bash ./build_www.sh
