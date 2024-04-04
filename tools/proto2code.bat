@echo off
rem Author: i0gan
rem Email : l418894113@gmail.com
rem Date  : 2023-01-04
rem Github: https://github.com/i0gan/Squick
rem Description: Generate proto code


set proto_bin=..\third_party\build\bin\protoc
set proto_path="..\src\proto"
set cpp_out_path="..\src\struct"
set csharp_out_path="..\client\proto\csharp"
set lua_out_path="..\client\proto\lua"

mkdir %csharp_out_path%

for %%f in (%proto_path%\*.proto) do  %proto_bin% --csharp_out=%csharp_out_path% --proto_path=%proto_path% %%f
rem windows dll于linux so有所区别，windows在需要声明导出函数，才能让其他dll或程序进行调用 参考 https://www.cnblogs.com/zhongpan/p/8378825.html
for %%f in (%proto_path%\*.proto) do  %proto_bin% --cpp_out=dllexport_decl="WIN_DLL_EXPORT":%cpp_out_path% --proto_path=%proto_path% %%f

rem 删除多余的proto
del /f /q /s %csharp_out_path%\N*.cs

rem 生成Lua文件
mkdir ..\src\lua\proto
python proto_enum_to_lua.py
python proto_to_lua_str.py

mkdir %lua_out_path%
copy ..\src\lua\proto\enum.lua %lua_out_path%