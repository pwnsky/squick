@echo off
rem 生成deploy文件

del /f /q /s  ..\deploy

cd ..
mkdir deploy\bin
mkdir deploy\config
mkdir deploy\src\proto
mkdir deploy\src\lua
mkdir deploy\data\logs
mkdir deploy\data\crash

xcopy /s /e /y bin deploy\bin
echo d | xcopy /s /e /y script deploy\script
xcopy /s /e /y config deploy\config
xcopy /s /e /y src\proto deploy\src\proto
xcopy /s /e /y src\lua deploy\src\lua

cd deploy\bin
del /f /q /s *.pdb
del /f /q /s *.exp
del /f /q /s *.lib

cd..
del /f /q /s *.sh
