
rem Author: i0gan
rem Email : l418894113@gmail.com
rem Date  : 2022-05-07
rem Github: https://github.com/pwnsky/squick
rem Description: Stop all tester script

cd /d %~dp0

@echo off

taskkill /f /im tester*
