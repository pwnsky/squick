@echo off
rem Author: i0gan
rem Email : l418894113@gmail.com
rem Date  : 2023-01-04
rem Github: https://github.com/pwnsky/squick
rem Description: Build sqkctl tools

set build_version="release"
set project_path=%~dp0\..
set build_path=%project_path%\cache

cd %project_path%
mkdir %build_path%\sqkctl
cd %build_path%\sqkctl
cmake %project_path%\src\tools\sqkctl
cmake --build . 

if "%1"=="no_pause" (
    echo continue
) else (
    pause
)