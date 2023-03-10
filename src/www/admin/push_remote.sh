#!/bin/bash

rsync -avz --delete ./dist/ root@tflash.pwnsky.com:/srv/t-flash/admin/www/
