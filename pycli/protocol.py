import struct
import sys
sys.path.append('./proto')

from proxy_pb2 import *
from web_pb2 import *
from player_pb2 import *
from test_pb2 import *
from game_pb2 import *
from base_pb2 import *
from msg_id_pb2 import *
from google.protobuf.json_format import MessageToJson


print("sys", sys.path)

def Encode(msg_id, data):
    send_data = b'' 
    send_data += (msg_id).to_bytes(2, byteorder="big", signed=False)
    send_data += (len(data) + 6).to_bytes(4, byteorder="big", signed=False)
    send_data += data
    return send_data

def Decode(recv_data):
    msg_id = int.from_bytes(recv_data[0:2], byteorder='big', signed=False)
    length = int.from_bytes(recv_data[2:6], byteorder='big', signed=False)
    return msg_id, recv_data[6:]
