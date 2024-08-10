import requests
import json

url = 'http://127.0.0.1:8888/api/execute_lua'

data = {
    'node_id': 1000,
    'script': 'return print_t(ModuleMgr)',
    'type': 0
}

global_env = '''
Lua debug console
The global table symble:
Env
ModuleMgr
'''
print(global_env)
print('Example: print_t(Env)')

def check_input(user_input):
    real_inpurt = ''
    if 'return' not in user_input:
       real_input = 'return ' + user_input
    return real_input
    

while True:
    user_input = input("lua: ")
    data['script'] = check_input(user_input)
    #print(user_input)
    response = requests.post(url, data=json.dumps(data))
    output = json.loads(response.text)
    print(output['output'])
