rem Author: i0gan
rem Email : l418894113@gmail.com
rem Date  : 2022-03-18
rem Github: https://github.com/pwnsky/squick
rem Description: Format all source files


set clang_format_bin=clang-format
set src_path="..\..\src"

set format_style="{IndentWidth: 4, ColumnLimit: 160}"

for /R %src_path% %%f in (*.proto) do %clang_format_bin% -i --style=google --style=%format_style% "%%f"
for /R %src_path% %%f in (*.cc) do    %clang_format_bin% -i --style=google --style=%format_style% "%%f"
for /R %src_path% %%f in (*.h)  do    %clang_format_bin% -i --style=google --style=%format_style% "%%f"

pause