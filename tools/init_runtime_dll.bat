@echo off
rem Author: i0gan
rem Email : l418894113@gmail.com
rem Date  : 2022-01-07
rem Github: https://github.com/pwnsky/squick
rem Description: Copy Windows dependency library

set project_path=%~dp0\..

mkdir %project_path%\data\logs
mkdir %project_path%\data\crash
mkdir %project_path%\bin
cd %project_path%\bin
xcopy /s /e /y ..\third_party\build\lib\*.dll .
copy ..\third_party\build\bin\*.dll .

if "%1"=="no_pause" (
    echo continue
) else (
    pause
)