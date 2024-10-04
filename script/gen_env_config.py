#! /usr/bin/python3
import os
import collections
import sys

if len(sys.argv) < 2:
    env='dev'
else:
    env=sys.argv[1]

print('Gen env: ' + env)

tmpl_path = '../config/tmpl'
tmpl_value_path = '../config/tmpl/env'
cfg_path = '../config/node'

cfg_map = {}
conf_content = ''

def GetFileContent(path):
    fd = open(path, 'r')
    return fd.read()

for root, dirs, files in os.walk(tmpl_path):
    for f in files:
        if f.endswith('.json'):
            print("json file: ", f)
            cfg_map[f] = GetFileContent(tmpl_path + '/' + f)
        elif f.endswith('.conf'):
            print("conf file: ", f)
            if f == env + '.conf':
                conf_content = GetFileContent(tmpl_path + '/env/' + f)


# replace
if (len(conf_content) > 0):
    conf_arr = conf_content.split('\n')
    for conf_line in conf_arr:
        conf = conf_line.split('=')
        if len(conf) < 2:
            continue
        key = conf[0]
        value = conf[1]
        for k2 in cfg_map:
            tmp = cfg_map[k2]
            tmp = tmp.replace('{' + key + '}', value)
            cfg_map[k2] = tmp 
else:
    print("Get " + env + " error")
    exit(-1)

# save
for k in cfg_map:
    fd = open(cfg_path + '/' + k, 'w')
    fd.write(cfg_map[k])
    fd.close()
