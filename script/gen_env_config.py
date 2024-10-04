#! /usr/bin/python3
import os
import collections
import sys

if len(sys.argv) < 2:
    env='dev'
else:
    env=sys.argv[1]

print('Gen env: ' + env)

package_list = {}
tmpl_path = '../config/tmpl'
tmpl_value_path = '../config/tmpl/env'
cfg_path = '../config/node'

cfg_map = {}
cfg_value_map = {}

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
            cfg_value_map[f] = GetFileContent(tmpl_path + '/env/' + f)


# replace
for k in cfg_value_map:
    conf_file = cfg_value_map[k]
    conf_arr = conf_file.split('\n')
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

# save
for k in cfg_map:
    fd = open(cfg_path + '/' + k, 'w')
    fd.write(cfg_map[k])
    fd.close()
