@echo off
rem Author: i0gan
rem Email : l418894113@gmail.com
rem Date  : 2022-12-14
rem Github: https://github.com/pwnsky/squick
rem Description: Build squick all files

set build_version="debug"
set project_path= %~dp0\..
set build_path=%project_path%\cache

mkdir %build_path%

call "proto2code.bat no_pause"
call "generate_config.bat no_pause"
call "init_runtime_dll.bat no_pause"

rem cmake 
cd %build_path%\
echo %project_path%
cmake %project_path% -DMODE=dev
cmake --build . -j8

if "%1"=="no_pause" (
    echo continue
) else (
    pause
)