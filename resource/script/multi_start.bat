rem Author: i0gan
rem Email : l418894113@gmail.com
rem Date  : 2022-11-27
rem Github: https://github.com/i0gan/Squick
rem Description: Start all servers script

cd bin

start cmd /c " squick type=master id=1 "
start cmd /c " squick type=login id=2 "
start cmd /c " squick type=world id=100 "
start cmd /c " squick type=world id=101 "
start cmd /c " squick type=db_proxy id=300 "
start cmd /c " squick type=db_proxy id=301 "

start cmd /c " squick type=lobby id=1000 "
start cmd /c " squick type=lobby id=1001 "
start cmd /c " squick type=lobby id=1002 "

rem start cmd /c " squick type=game_mgr id=2000 "

start cmd /c " squick type=proxy id=500 "
start cmd /c " squick type=proxy id=501 "
start cmd /c " squick type=proxy id=502 "