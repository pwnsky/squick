@echo off
rem Author: i0gan
rem Email : l418894113@gmail.com
rem Date  : 2023-10-01
rem Github: https://github.com/pwnsky/squick
rem Description: Clone windows third party lib and init

set project_path= %~dp0\..
cd %project_path%
git submodule update --init
cd %project_path%\res
git reset --hard .
cd %project_path%

git clone https://github.com/pwnsky/squick-thirdparty-build.git
del /f /q /s  %project_path%\third_party\build
mkdir %project_path%\third_party\build
xcopy /s /e /y %project_path%\squick-thirdparty-build\Windows\build %project_path%\third_party\build
cd %project_path%
del /f /q /s  %project_path%\squick-thirdparty-build

if "%1"=="no_pause" (
    echo continue
) else (
    pause
)