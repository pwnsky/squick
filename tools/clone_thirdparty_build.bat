git clone https://github.com/pwnsky/squick-thirdparty-build.git
del /f /q /s  ..\third_party\build
mkdir ..\third_party\build
xcopy /s /e /y squick-thirdparty-build\Windows\build ..\third_party\build
del /f /q /s  ..\squick-thirdparty-build
