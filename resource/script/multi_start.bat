
rem Author: i0gan
rem Email : l418894113@gmail.com
rem Date  : 2022-11-27
rem Github: https://github.com/i0gan/Squick
rem Description: Start all servers script

cd bin

start cmd /c " squick plugin=master.xml server=master id=1 "
start cmd /c " squick plugin=db_proxy.xml server=db_proxy id=4 "
start cmd /c " squick plugin=world.xml server=world id=2 "
start cmd /c " squick plugin=login.xml server=login id=3 "
start cmd /c " squick plugin=gateway.xml server=gateway id=5 "

start cmd /c " squick plugin=game.xml server=game id=10 "
start cmd /c " squick plugin=game.xml server=game id=11 "

start cmd /c " squick plugin=gameplay_manager.xml server=gameplay_manager id=20 "

start cmd /c " squick plugin=proxy.xml server=proxy id= 6"
