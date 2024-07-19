from protocol import *

Instance = {}

def RegisterMsg(instance):
    global Instance
    print("Register msg")
    if ('callback' not in instance):
        instance['callback'] = {}
    instance['callback'][IdAckConnectProxy] = HandleAckConnectProxy
    instance['callback'][IdAckPlayerEnter] = HandleAckPlayerEnter
    instance['callback'][IdAckPlayerData] = HandleAckPlayerData
    Instance = instance


def SendMsg(msg_id, pb_msg):
    data = Encode(msg_id, pb_msg.SerializeToString())
    Instance['ws'].send(data, 2)

def HandleAckConnectProxy(msg_id, msg):
    print("Connected ...")
    SendMsg(IdReqPlayerEnter, ReqConnectProxy())

def HandleAckPlayerEnter(msg_id, msg):
    ack = AckPlayerEnter()
    ack.ParseFromString(msg)
    if ack.code == ErrCommonSucc:
        print("Player entered ...")
        SendMsg(IdReqPlayerData, ReqPlayerData())
    else:
        print("Error")

def HandleAckPlayerData(msg_id, msg):
    print("okkkk")