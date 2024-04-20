@echo off
rem Author: i0gan
rem Email : l418894113@gmail.com
rem Date  : 2023-03-10
rem Github: https://github.com/pwnsky/squick
rem Description: Clean all builded files

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
del /f /q /s lua\struct