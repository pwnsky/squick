#! /bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2023-02-11
# Github: https://github.com/i0gan/Squick
# Description: Upload deploy files to remote server

bash generate_deploy.sh

echo "Uploading ..."

remote_ip=1.14.123.62
remote_user=root

ssh $remote_user@$remote_ip "cd /srv/action && bash stopall.sh"
rsync -avz --delete ../deploy/ $remote_user@$remote_ip:/srv/action
ssh $remote_user@$remote_ip "tmux send -t action \"cd /srv/action && bash startall.sh \" ENTER"

echo "Uploaded"

sleep 1
