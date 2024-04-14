@echo off
rem Author: i0gan
rem Email : l418894113@gmail.com
rem Date  : 2023-10-01
rem Github: https://github.com/pwnsky/squick
rem Description: Clone windows third party lib

git clone https://github.com/pwnsky/squick-thirdparty-build.git
del /f /q /s  ..\third_party\build
mkdir ..\third_party\build
xcopy /s /e /y squick-thirdparty-build\Windows\build ..\third_party\build
del /f /q /s  ..\squick-thirdparty-build
