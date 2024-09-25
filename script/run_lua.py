#! /bin/python
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
example1='''
Example 1: print env info

print_t(Env)
:r
'''
print(example1)

example2='''
Example2: print the config data

local ta = Squick:GetConfig(Excel.Scene.ThisName)
return print_t(ta)
:r
'''
print(example2)

print("input :r to run")
print("input :q to quit")

def check_input(user_input):
    real_inpurt = ''
    if 'return' not in user_input:
       real_input = 'return ' + user_input
    else:
        real_input = user_input
    return real_input
    

while True:
    script = ""
    while True:
        user_input = input()
        if user_input == ':r':
            break
        if user_input == ':q':
            exit()
        script += user_input + '\n'
    print(script)
    data['script'] = check_input(script)
    #print(user_input)
    response = requests.post(url, data=json.dumps(data))
    output = json.loads(response.text)
    print(output['output'])
