@echo off
rem Author: i0gan
rem Email : l418894113@gmail.com
rem Date  : 2022-11-27
rem Github: https://github.com/pwnsky/squick
rem Description: Start all servers script

call env.bat
python gen_env_config.py %SQUICK_RUN_ENV%

cd ..\bin

start /b .\squick