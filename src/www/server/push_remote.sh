#!/bin/bash

rm ./build/tflash.exe
ssh root@tflash.pwnsky.com "pkill tflash"
rsync -avz --delete ./build/ root@tflash.pwnsky.com:/srv/t-flash/server/
ssh root@tflash.pwnsky.com "tmux send -t t-flash \"cd /srv/t-flash/server && ./tflash http :80 &\" ENTER"
