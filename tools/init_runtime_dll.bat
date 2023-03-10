rem Author: i0gan
rem Email : l418894113@gmail.com
rem Date  : 2022-01-07
rem Github: https://github.com/i0gan/Squick
rem Description: Copy Windows dependency library

set project_path=%~dp0\..

cd %project_path%\bin
xcopy /s /e /y lib .
copy ..\third_party\build\bin\libprotobufd.dll .
copy ..\third_party\build\lib\event.dll event.dll
copy ..\third_party\build\lib\event_core.dll event_core.dll
copy ..\third_party\build\lib\event_extra.dll event_extra.dll
pause