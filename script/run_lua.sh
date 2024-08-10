#! /bin/bash

API_URL="http://127.0.0.1:8888/api/execute_lua"
API_SALT=pwnsky_squick

node_id=1000
code='return print_t(ModuleMgr)'

curl -X POST $API_URL -d "{\"node_id\" : $node_id, \"script\": \"$code\", \"type\": 1}"
