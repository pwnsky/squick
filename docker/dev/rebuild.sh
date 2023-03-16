#! /bin/bash

cd /mnt/tools
bash ./proto2code.sh
bash ./generate_config.sh
bash ./build_squick.sh
bash ./build_server.sh
#bash ./build_www.sh