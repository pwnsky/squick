@echo off
rem 删除缓存以及编译出来的文件
cd ..
del /f /q /s  client
del /f /q /s  cache
del /f /q /s  bin
del /f /q /s  config\struct\*
del /f /q /s  config\proto\*
del /f /q /s  config\ini\*
cd src\struct
del /f /q /s *.pb.h
del /f /q /s *.pb.cc
del /f /q /s excel.h

cd ..\..\
del /f /q /s lua\proto
