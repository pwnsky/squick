# python3
import requests
import requests.utils
import http.cookiejar as cookiejar
import json
import websocket
import _thread
import time
import rel
# pip install websocket-client websocket rel 
# pip install requests
# pip install http

print("pycli:")

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
        return True, None
    except Exception as ex:
        print(ex)
        return True, None

def on_message(ws, message):
    print(message)

def on_error(ws, error):
    print(error)

def on_close(ws, close_status_code, close_msg):
    print("### closed ###")

def on_open(ws):
    print("Opened connection")


if __name__ == '__main__':
    # login
    err, rsp = login("pycli", "password")
    if (err):
        exit()
    
    print("login rsp: " + rsp)
    jrsp = json.loads(rsp)
    wsUrl = 'ws://' + str(jrsp['ip']) + ':' + str(jrsp['ws_port']) + '/'
    print("connect to proxy: " + wsUrl)
    ws = websocket.WebSocketApp(wsUrl,
                              on_open=on_open,
                              on_message=on_message,
                              on_error=on_error,
                              on_close=on_close)

    rel.signal(2, rel.abort)  # Keyboard Interrupt
    rel.dispatch()

    # Set dispatcher to automatic reconnection, 5 second reconnect delay if connection closed unexpectedly
    ws.run_forever(dispatcher=rel, reconnect=5)