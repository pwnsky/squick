@echo off
rem Author: i0gan
rem Email : l418894113@gmail.com
rem Date  : 2022-05-07
rem Github: https://github.com/pwnsky/squick
rem Description: start 100 testers to test server

cd bin

for /l %%i in (1,1,100) do (
echo start tester_%%i
start .\tester type=tester test=proxy id=100%%i
)


pause