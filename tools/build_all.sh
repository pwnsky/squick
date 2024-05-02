#! /bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2022-11-27
# Github: https://github.com/pwnsky/squick
# Description: build script for squick
cd $(dirname $0)
bash ./clean_all.sh
bash ./build_third_party.sh
bash ./build_sqkctl.sh
bash ./proto2code.sh
bash ./generate_config.sh
bash ./build_squick.sh
