# python3
import requests
import requests.utils
import http.cookiejar as cookiejar
import json
import websocket
import _thread
import time
import rel
import sys
from protocol import *
from logic import *

Instance = {}

session = requests.session()
session.cookies = cookiejar.LWPCookieJar(filename='./login.cookie')
BaseUrl = 'http://127.0.0.1:8088'
header = {
    'Referer': BaseUrl + "login",
    'User-Agent': 'Mozilla/5.0 (Windows NT 6.1; Win64; x64) Chrome/89.0.4389.82'
}

def login(account, password):
    try:
        login_data = {
            'LoginType': 0,
            'account': account,
            'password': password,
            'device' : 'pycli'
        }
        resp = session.post(url=BaseUrl + '/login', headers=header, data=json.dumps(login_data))
        if resp.status_code == requests.codes.ok:
            cookie = requests.utils.dict_from_cookiejar(session.cookies)
            session.cookies.save(ignore_discard=True, ignore_expires=True)
            return False, resp.text
        print("Error: ErrorCode: ", resp.status_code)
        return True, None
    except Exception as ex:
        print("Error:", ex)
        return True, None


def AuthConnection(ws):
    print("Begin auth")
    req = ReqConnectProxy()
    req.account_id = Instance['login']['account_id']
    req.key = Instance['login']['key']
    req.login_node = Instance['login']['login_node']
    req.signatrue = Instance['login']['signatrue']
    sdata = req.SerializeToString()
    data = Encode(IdReqConnectProxy, sdata)
    ws.send(data, 2)

def HandleMsg(data):
    msg_id, msg = Decode(data)
    Instance['callback'][msg_id](msg_id, msg)

def OnWsRecv(ws, message):
    print("Message:", message)
    HandleMsg(message)

def OnWsError(ws, error):
    print("Error:", error)

def OnWsClose(ws, close_status_code, close_msg):
    print("### closed ###")

def OnWsOpen(ws):
    print("Opened connection")
    AuthConnection(ws)
    

if __name__ == '__main__':
    # login
    print("Begin pycli")
    err, rsp = login("pycli_1", "password")
    if (err):
        print("Error...")
        exit()
    
    print("login rsp: " + rsp)
    jrsp = json.loads(rsp)
    wsUrl = 'ws://' + str(jrsp['ip']) + ':' + str(jrsp['ws_port']) + '/'
    global LoginInfo
    Instance['login'] = jrsp
    print("connect to proxy: " + wsUrl)
    ws = websocket.WebSocketApp(wsUrl,
                              on_open=OnWsOpen,
                              on_message=OnWsRecv,
                              on_error=OnWsError,
                              on_close=OnWsClose)
    Instance['ws'] = ws
    RegisterMsg(Instance)
    # Set dispatcher to automatic reconnection, 5 second reconnect delay if connection closed unexpectedly
    ws.run_forever(dispatcher=rel, reconnect=5)

    rel.signal(2, rel.abort)  # Keyboard Interrupt
    rel.dispatch()
    print("run")
