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


print("sys", sys.path)

def Encode(msg_id, data):
    send_data = b'' 
    send_data += (msg_id).to_bytes(2, byteorder="big", signed=False)
    send_data += (len(data)).to_bytes(4, byteorder="big", signed=False)
    send_data += data
    return send_data

def Decode(recv_data):
    return msg_id, data

