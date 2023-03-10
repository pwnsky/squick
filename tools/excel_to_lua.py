#! /usr/bin/python3
#encoding=utf-8

# pip install xlrd==1.2.0

import os
import xlrd
import argparse
import sys

inpath = ''
outpath = ''

head_file_code = '''-------------------------------------------------------------------------------
-- @file: %(filename)s
-- @desc: don't edit this file!!!!
-------------------------------------------------------------------------------

%(tablename)s = {
%(configs)s
}
'''


key_word_replace = {
    'function': '["function"]',
    'end': '["end"]',
    'for': '["for"]',
    'do': '["do"]',
    'if': '["if"]',
    'else': '["else"]',
    'elseif': '["elseif"]',
    'then': '["then"]',
    'while': '["while"]',
}

ignore_config_file = {
    'AllLocalizationWords.xlsx': True,
    'Drop_Outside_CardShow.xlsx': True,
    'Drop_Outside_Show.xlsx': True,
    'Help_Text.xlsx': True,
    'Localization.xlsx': True,
}

def generate_lua_file(filename, data):
    print('start generate file:' + outpath +'/'+ filename)
    table_name = filename[: filename.find('.lua')]
    with open(outpath + '/'+ filename, 'w+', encoding='utf-8') as out:
        configs = ''
        row_prefix = ''
        for k,v in data.items():
            row_str = row_prefix + '[' + str(k) + '] = {\n'
            line_str = '  '
            for _k, _v in v.items():
                key = str(_k)
                if key.startswith('remark'):
                    continue
                kk = key_word_replace.get(key)
                if kk is not None:
                    _k = kk
                content = str(_v)
                if len(content) > 0:
                    this_line_str = str(_k) + '=' + content + ','
                    if len(line_str + this_line_str) >= 100:
                        row_str += line_str + '\n'
                        line_str = '  ' + this_line_str
                    else:
                        line_str += this_line_str
            row_str += line_str
            row_str += '\n}'
            row_prefix = ',\n'

            configs += row_str

        out.write(head_file_code % {'filename': filename, 'tablename': table_name,
                                    'configs': configs})

def parse_xls_file(filename):
    for fn, _ in ignore_config_file.items():
        if filename.find(fn) >= 0:
            print('ignore ' + filename)
            return
    print('start parsing file:' + inpath + ' / ' + filename)
    luafilename = os.path.basename(filename)
    luafilename = luafilename[: luafilename.find('.xlsx')] + '.lua'

    luafilename = 't_' + luafilename.lower()
    data = xlrd.open_workbook(filename)
    table = data.sheets()[0]

    field_desc = []
    for i in range(0, table.ncols):
        key = table.cell_value(0, i)
        if key.find('KEY') >= 0:
            field_desc.append({'key': key, 'is_key': True})
        else:
            field_desc.append({ 'key': key })

    for i in range(0, table.ncols):
        field_desc[i]['field_type'] = table.cell_value(3, i)

    parse_result = {}
    for i in range(4, table.nrows):
        row_data = {}
        key_data = None
        for j in range(0, table.ncols):
            if field_desc[j].get('is_key') is not None:
                field_type = field_desc[j]['field_type']
                if field_type == 'int':
                    if table.cell_value(i, j) == '':
                        key_data = 0
                    else:
                        key_data = int(table.cell_value(i, j))
                else:
                    data = str(table.cell_value(i, j))
                    data = data.replace('\\', '\\\\')
                    key_data = '"' + data + '"'
            else:
                field_type = field_desc[j]['field_type']
                field_key = field_desc[j]['key']
                value = table.cell_value(i,j)
                if field_type == 'string' or field_type == 'localize':
                    cell_data = str(table.cell_value(i,j))
                    if cell_data.endswith(']'):
                        row_data[field_key] = '\'' + str(cell_data).replace('\'', '"') + '\''
                    else:
                        row_data[field_key] = '[[' + str(table.cell_value(i, j)) + ']]'
                elif field_type.find('array') >= 0:
                    if field_type.find('int_array') >= 0 or field_type.find('float_array') >= 0:
                        is_int_array = False
                        if field_type.find('int_array') >= 0:
                            is_int_array = True
                        row_data[field_key] = value.replace('[', '').replace(']','')
                        elements = row_data[field_key].split(',')
                        elements_str = '{'
                        prefix = ''
                        for idx, e in enumerate(elements):
                            if e.find('M') > 0:
                                if is_int_array:
                                    v = int(float(e[: e.find('M')]) * 1000000)
                                else:
                                    v = float(e[: e.find('M')]) * 1000000
                                elements_str += prefix + str(v)
                            else:
                                elements_str += prefix + e
                            prefix = ','
                        elements_str += '}'
                        row_data[field_key] = elements_str
                    else:
                        row_data[field_key] = value.replace('[', '{').replace(']','}')
                        if row_data[field_key] == None or row_data[field_key] == '':
                            row_data[field_key] = '{}'
                elif field_type == 'int':
                    try:
                        mulit = 1
                        value = str(value)
                        if value.find('M') >= 0:
                            value = value[: value.find('M')]
                            mulit = 1000000
                        row_data[field_key] = int(float(value) * mulit)
                    except Exception as e:
                        row_data[field_key] = 0
                elif field_type == 'float':
                    try:
                        mulit = 1
                        value = str(value)
                        if value.find('M') >= 0:
                            value = value[: value.find('M')]
                            mulit = 1000000
                        row_data[field_key] = float(value) * mulit
                    except Exception as e:
                        row_data[field_key] = 0
                else:
                    row_data[field_key] = value

        parse_result[key_data] = row_data
    generate_lua_file(luafilename, parse_result)


def generate_lua_file_loca(filename, data):
    print('start generate file:' + outpath +'/'+ filename)
    table_name = filename[: filename.find('.lua')]
    with open(outpath + '/'+ filename, 'w+', encoding='utf-8') as out:
        configs = ''
        row_prefix = ''
        for k,v in data.items():
            row_str = row_prefix + '["' + str(k) + '"] = {\n'
            line_str = '  '
            for _k, _v in v.items():
                #kk = key_word_replace.get(key)
                #if kk is not None:
                #    _k = kk
                content = str(_v)
                if len(content) > 0:
                    this_line_str = str(_k) + '=' + content + ','
                    if len(line_str + this_line_str) >= 100:
                        row_str += line_str + '\n'
                        line_str = '  ' + this_line_str
                    else:
                        line_str += this_line_str
            row_str += line_str
            row_str += '\n}'
            row_prefix = ',\n'

            configs += row_str

        out.write(head_file_code % {'filename': filename, 'tablename': table_name,
                                    'configs': configs})

def parse_localization_file(filename):
    luafilename = os.path.basename(filename)
    luafilename = luafilename[: luafilename.find('.xlsx')] + '.lua'

    luafilename = 't_' + luafilename.lower()
    data = xlrd.open_workbook(filename)
    table = data.sheets()[0]

    field_desc = []
    for i in range(0, table.ncols):
        key = table.cell_value(0, i)
        if key.find('Key') >= 0:
            field_desc.append({'key': key, 'is_key': True})
        else:
            field_desc.append({ 'key': key })

    for i in range(0, table.ncols):
        field_desc[i]['field_type'] = 'string'

    parse_result = {}
    for i in range(1, table.nrows):
        row_data = {}
        key_data = None
        for j in range(0, table.ncols):
            if field_desc[j].get('is_key') is not None:
                key_data = str(table.cell_value(i,j))
            else:
                field_key = field_desc[j].get('key')
                cell_data = str(table.cell_value(i,j))
                if cell_data is not None and cell_data != '':
                    if cell_data.endswith(']'):
                        row_data[field_key] = '\'' + str(table.cell_value(i, j)).replace('\'', '"') + '\''
                    else:
                        row_data[field_key] = '[[' + str(table.cell_value(i, j)) + ']]'
        parse_result[key_data] = row_data
    generate_lua_file_loca(luafilename, parse_result)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='export xls')
    parser.add_argument('-i', '--input', dest='input', help='xls path')
    parser.add_argument('-o', '--out', dest='out', help='lua file path')

    args = parser.parse_args()
    if args.input is None or len(args.input) == 0:
        inpath = '../resource/lua_excel'
    else:
        inpath = args.input

    if args.out is None or len(args.out) == 0:
        outpath = '../src/lua/config'
    else:
        outpath = args.out

    for root, dirs, fs in os.walk(inpath):
        for f in fs:
            if f.endswith('.xlsx'):
                parse_xls_file(os.path.join(root, f))
    parse_localization_file(os.path.join(inpath, 'AllLocalizationWords.xlsx'))
