@echo off
rem Author: i0gan
rem Email : l418894113@gmail.com
rem Date  : 2022-05-07
rem Github: https://github.com/pwnsky/squick
rem Description: start 100 testers to test server

cd ..\bin

for /l %%i in (1,1,100) do (
echo start tester_%%i
start /b .\sqkcli type=sqkcli test=proxy ip=127.0.0.1 port=10501 hide=true id=100%%i
)
