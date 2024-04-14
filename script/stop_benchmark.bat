@echo off
rem Author: i0gan
rem Email : l418894113@gmail.com
rem Date  : 2023-05-07
rem Github: https://github.com/pwnsky/squick
rem Description: Stop benchmark test

cd /d %~dp0

@echo off

taskkill /f /im sqkcli*
