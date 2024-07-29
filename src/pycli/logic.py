from protocol import *
import threading
import time

Instance = {}
IsExit = False

def LogicExit():
    print("Logic exit")
    global IsExit;
    IsExit = True

def RegisterMsg(instance):
    global Instance
    print("Register msg")
    if ('callback' not in instance):
        instance['callback'] = {}
    instance['callback'][IdAckConnectProxy] = HandleAckConnectProxy
    instance['callback'][IdAckHeartBeat] = HandleAckHeartBeat
    instance['callback'][IdAckPlayerEnter] = HandleAckPlayerEnter
    instance['callback'][IdAckPlayerData] = HandleAckPlayerData
    Instance = instance
    Instance['is_connected'] = True
    Instance['last_heartbeat_time'] = 0
    t1 = threading.Thread(target=LogicLoop)
    t1.start()

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
    ack = AckPlayerData()
    ack.ParseFromString(msg)
    data = MessageToJson(ack)
    print("Get the player data: \n", data)


def HandleOffline(msg_id, msg):
    print("Player offline")

def HandleAckHeartBeat(msg_id, msg):
    ack = AckHeartBeat()
    ack.ParseFromString(msg)
    data = MessageToJson(ack)
    print("Get the heartbeat info: \n", data)
    index = 0

def LogicLoop():
    global Instance
    index = 0
    while IsExit == False:
        time.sleep(0.1)
        now_time = int(time.time())
        if Instance['is_connected'] == True:
            if(now_time - Instance['last_heartbeat_time'] > 5):
                Instance['last_heartbeat_time'] = now_time
                req = ReqHeartBeat()
                req.index = index
                SendMsg(IdReqHeartBeat, req)
                index += 1
