@echo off
rem Author: i0gan
rem Email : l418894113@gmail.com
rem Date  : 2022-11-27
rem Github: https://github.com/i0gan/Squick
rem Description: Generate configuration files

set config_path=..\config
set config_path_gen=../config
set excel_path=..\resource\excel
set excel_path_gen=../resource/excel
set struct_path=..\src\struct
set lua_proto_path=
set client_config_path=..\client
set lua_src_path=..\src\lua

mkdir %config_path%\excel
mkdir %config_path%\struct
mkdir %config_path%\ini

.\bin\sqkctl excel %excel_path_gen% %config_path_gen%

rem 拷贝 \proto\excel.h 
copy ..\config\excel\excel.h %struct_path%

mkdir %client_config_path%\ini
mkdir %client_config_path%\excel
mkdir %client_config_path%\struct
mkdir %client_config_path%\lua
mkdir %client_config_path%\csharp

xcopy /s /e /y %config_path%\ini %client_config_path%\ini
xcopy /s /e /y %config_path%\struct %client_config_path%\struct

xcopy /s /e /y %config_path%\excel %client_config_path%\excel
rd /s/q %config_path%\excel