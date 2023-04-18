@echo off
rem Author: i0gan
rem Email : l418894113@gmail.com
rem Date  : 2022-03-10
rem Github: https://github.com/pwnsky/Squick
rem Description: Register squick envirnment

echo "请采用管理员权限运行"

cd /d %~dp0
cd ..
set squick_path=%cd%
setx squick_path %squick_path%

set My_PATH=%squick_path%/tools/bin
set PATH=%PATH%;%My_PATH%
reg add "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v "Path" /t REG_EXPAND_SZ /d "%PATH%" /f


pause