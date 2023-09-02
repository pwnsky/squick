#! /usr/bin/python3
# encoding=utf-8
import os
import collections
import time
parser_result = {}

package_list = {}

def generate_lua_file():
    with open('../src/lua/proto/enum.lua', 'w+', encoding='utf-8') as out:
        out.write('''----------------------------------------------------------------------------------
-- don't edit it, generated from .proto files by tools
----------------------------------------------------------------------------------

''')
        for p, es in package_list.items():
            for e, k in es.items():
                out.write(e + ' = {\n')
                enum_value_str = '  '
                this_line_str = enum_value_str
                sortedx = collections.OrderedDict(sorted(k.items(), key=lambda x: int(x[1], 0)))
                for _k,_v in sortedx.items():
                    tmp_str = _k + ' = ' + _v + ', '
                    if len(this_line_str) + len(tmp_str) >= 100:
                        tmp_str = '\n  ' + tmp_str
                        this_line_str = tmp_str
                    else:
                        this_line_str += tmp_str
                    enum_value_str += tmp_str
                out.write(enum_value_str)
                out.write('\n}\n\n')

def add_enum_value(p, e, n, v):
    p = p.strip()
    e = e.strip()
    n = n.strip()
    v = v.strip()
    k = p # package name不写进去了
    '''k = ''
    if k != '':
        k = k + '.'
    if e != '':
        k = k + e + '.'
    k = k + n'''
    k = n

    if k.startswith('//'):
        return
    k = k.replace(e + '_', '')
    if p not in package_list:
        package_list[p] = {}
    if e not in package_list[p]:
        package_list[p][e] = {}

    package_list[p][e][k] = v
    print('enum =' + k + ', value=' + v)

def parse_proto(f):
    print('parse file:' + f)
    with open(f, 'r', encoding='utf-8') as inp:
        parse_package = True
        package_name = ''
        enum_name = ''
        parse_enum_name = False
        for line in inp.readlines():
            if parse_package is True:
                pos = line.find('package')
                if pos >= 0:
                    package_name = line[pos + len('package'): line.find(';')]
                    package_name = package_name.strip()
                    parse_package = False
                    parse_enum_name = True
            elif parse_enum_name:
                pos = line.find('enum')
                if pos >= 0:
                    enum_name = line[pos + len('enum') : line.find('{')]
                    parse_enum_name = False
            else:
                tmp = line.strip()
                if len(tmp) == 0:
                    continue
                if tmp[0] == '}':
                    parse_enum_name = True
                else:
                    if line.find('=') > 0:
                        values = line.split('=')
                        name = values[0].strip()
                        v = values[1]
                        v = v[: v.find(';')]

                        add_enum_value(package_name, enum_name, name, v.strip())


for root, dirs, files in os.walk('../src/proto/'):
    for f in files:
        if f.endswith('.proto'):
            parse_proto(os.path.join(root, f))

generate_lua_file()