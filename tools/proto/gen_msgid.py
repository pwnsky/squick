#! /usr/bin/python3
# encoding=utf-8
import os
import collections
import time
from datetime import datetime
parser_result = {}

package_list = {}
project_path = '../..'
current_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

file_desc='''
//----------------------------------------------------------------------------------
// don't edit it, generated from .proto files by tools
// ---------------------------------------------------------------------------------
// Update time: ''' + str(current_time) + "\n"

def generate_proto_file():
    with open(project_path + '/src/proto/msg_id.proto', 'w+', encoding='utf-8') as out:
        out.write(file_desc)
        out.write('''
syntax = "proto3";
package rpc;
enum MsgID {
NoneMsgID = 0;
''')
        for msg_id, value in parser_result.items():
            if value["is_node_proto"]:
                continue
            name = value["name"]
            desc = value["desc"]
            print("msg_id: " + str(msg_id) + " msg_name: " + name)
            line = "    ID" + name + " = " + str(msg_id) + "; // message: " + name + " desc: " + desc + "\n"
            out.write(line)
        out.write("}")

def generate_node_proto_file():
    with open(project_path + '/src/proto/n_msg_id.proto', 'w+', encoding='utf-8') as out:
        out.write(file_desc)
        out.write('''
syntax = "proto3";
package rpc;
enum NMsgID {
NoneNMsgID = 0;
''')
        for msg_id, value in parser_result.items():
            if value["is_node_proto"] == False:
                continue
            name = value["name"]
            desc = value["desc"]
            print("msg_id: " + str(msg_id) + " msg_name: " + name)
            line = "    ID" + name + " = " + str(msg_id) + "; // message: " + name + " desc: " + desc + "\n"
            out.write(line)
        out.write("}")


def generate_cpp_file():
    with open(project_path + '/src/struct/msg_id.cc', 'w+', encoding='utf-8') as cc_file:
        cc_file.write(file_desc)
        print("ok")

    with open(project_path + '/src/struct/msg_id.h', 'w+', encoding='utf-8') as h_file:
        h_file.write(file_desc)
        h_file.write('''
std::string GetMessageNameByID();
''')
        print("ok")

def parse_proto(f, is_node_proto):
    print('parse file:' + f)
    with open(f, 'r', encoding='utf-8') as inp:
        message_name = ''
        parse_message_name = True
        for line in inp.readlines():
            i = 0
            while i < 1: # Just do once
                i += 1
                # msg name
                msg_name_start_pos = line.find('message')
                if msg_name_start_pos < 0:
                    break
                msg_name_end_pos = line.find(' {')
                if msg_name_end_pos < 0:
                    break
                msg_name = line[msg_name_start_pos + len('message ') : msg_name_end_pos]
                
                # msg_id
                msg_id_start_pos = line.find('msg_id=')
                if msg_id_start_pos < 0:
                    break
                msg_id_end_pos = line.find(';')
                if msg_id_end_pos < 0:
                    break
                message_id = line[msg_id_start_pos + len('msg_id=') : msg_id_end_pos]
                print("msg name: [" + msg_name + "] msg_id=[" + message_id + "]")

                # msg desc
                msg_desc = ''
                sub_line = line[msg_id_end_pos + 1:-1]
                msg_desc_start_pos = sub_line.find('desc=')
                if msg_desc_start_pos >= 0:
                    msg_desc_end_pos = sub_line.find(';')
                    if msg_desc_end_pos >= 0:
                        msg_desc = sub_line[msg_desc_start_pos + len('desc=') : msg_desc_end_pos]

                parser_result[int(message_id)] = { "name" : msg_name, "desc" : msg_desc, "is_node_proto" : is_node_proto}

for root, dirs, files in os.walk(project_path + '/src/proto/'):
    for f in files:
        if f.endswith('.proto'):
            is_node_proto = False
            if f.startswith('n_'):
                is_node_proto = True
            parse_proto(os.path.join(root, f), is_node_proto)

generate_proto_file()
generate_node_proto_file()
generate_cpp_file()