@echo off
rem Author: i0gan
rem Email : l418894113@gmail.com
rem Date  : 2022-11-27
rem Github: https://github.com/pwnsky/squick
rem Description: Stop all nodes

cd /d %~dp0

@echo off

taskkill /f /im squick*
