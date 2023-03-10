-----------------------------------------------------------------------------
-- Author: i0gan
-- Email : l418894113@gmail.com
-- Date  : 2022-12-04
-- Github: https://github.com/i0gan/Squick
-- Description: 打印table
-----------------------------------------------------------------------------

function print_table(table)
	if table == nil then
		print("the table is nil");
		print(debug.traceback())
		return;
	end
	
	local key = ""
	level =  1
	local indent = ""
	for i = 1, level do
	indent = indent.."  "
	end
	
	if key ~= "" then
	print(indent..key.." ".."=".." ".."{")
	else
	print(indent .. "{")
	end
	
	key = ""
	for k,v in pairs(table) do
	if type(v) == "table" then
		key = k
		print(indent .. key .. " =")
		print_table(v, level + 1)
	else
		local content = string.format("%s%s = %s", indent .. "  ",tostring(k), tostring(v))
		print(content..";")
		end
	end
	print(indent .. "}")

end