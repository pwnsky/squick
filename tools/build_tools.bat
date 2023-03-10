rem Author: i0gan
rem Email : l418894113@gmail.com
rem Date  : 2023-01-04
rem Github: https://github.com/i0gan/Squick
rem Description: Build tools

set build_version="release"
set project_path=%~dp0\..
set build_path=%project_path%\cache

cd %project_path%
mkdir %build_path%\tools
cd %build_path%/tools
cmake %project_path%\src\tools\xlsx2need
cmake --build . 
copy Debug\xlsx2need.exe ..\..\tools
pause