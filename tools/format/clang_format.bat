rem Author: i0gan
rem Email : l418894113@gmail.com
rem Date  : 2022-03-18
rem Github: https://github.com/pwnsky/squick
rem Description: Format all source files


set clang_format_bin=clang-format
set src_path="..\..\src"

for /R %src_path% %%f in (*.proto) do   %clang_format_bin% -i --style=google --style="{ColumnLimit: 160}" "%%f"
rem for /R %src_path% %%f in (*.cc) do  %clang_format_bin% -i --style=google --style="{ColumnLimit: 160}" "%%f"
rem for /R %src_path% %%f in (*.h)  do  %clang_format_bin% -i --style=google --style="{ColumnLimit: 160}" "%%f"

pause