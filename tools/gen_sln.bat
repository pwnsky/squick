@echo off
rem Author: i0gan
rem Email : l418894113@gmail.com
rem Date  : 2023-03-10
rem Github: https://github.com/pwnsky/squick
rem Description: Generate visual studio project

set build_version="debug"
set project_path= %~dp0\..
set build_path=%project_path%\cache
mkdir %build_path%
rem Gen config files

cd %project_path%\res
call ".\GenProtoCode.bat" server
call ".\ExportTable.bat" server

cd %project_path%\tools
call "init_runtime_dll.bat" no_pause

rem cmake 
cd %build_path%\
echo %project_path%
cmake %project_path% -DMODE=dev

rem Project has generated at %build_path%
explorer ..\cache
pause