#! /usr/bin/python3
# Gen proto files to lua files, just for client
import os
import collections

parser_result = {}

package_list = {}
proto_str = ""
is_find_package = False
is_find_syntax  = False
variable_name = "proto_code"
project_path = '../..'
def generate_lua_file():
    with open( project_path + '/client/proto/lua/proto.lua', 'w+', encoding='utf-8') as out:
        out.write('''----------------------------------------------------------------------------------
-- don't edit it, generated from .proto files by tools
----------------------------------------------------------------------------------

''')
        out.write(variable_name + " = [[\n" + proto_str + "\n]]")


def parse_proto(f):
    global proto_str, is_find_package, is_find_syntax
    print('parse file:' + f)
    with open(f, 'r', encoding='utf-8') as inp:
        for line in inp.readlines():
                pos = line.find('package')
                if pos >= 0:
                    if is_find_package == False:
                        proto_str += line
                        is_find_package = True
                    continue
                pos = line.find('syntax')
                if pos >= 0:
                    if is_find_syntax == False:
                        proto_str += line
                        is_find_syntax = True
                    continue
                pos = line.find('import')
                if pos >= 0:
                    continue
                proto_str += line

for root, dirs, files in os.walk(project_path + '/src/proto/'):
    for f in files:
        file_name = os.path.basename(f)
        if f.endswith('.proto') and f.startswith("n_") == False:
            parse_proto(os.path.join(root, f))
generate_lua_file()
