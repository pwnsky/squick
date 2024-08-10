#! /bin/bash
# reload json config

API_URL="http://127.0.0.1:8888/api/reload"
API_SALT=pwnsky_squick


echo -e "\nreloading node config"
sleep 1
curl $API_URL?type=node


echo -e "\nreloading xml config"
sleep 1
curl $API_URL?type=xml


echo -e "\nreloading lua script"
sleep 1
curl $API_URL?type=lua

echo -e "\nall config and script reloaded"
