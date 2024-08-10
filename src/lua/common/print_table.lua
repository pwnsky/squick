-----------------------------------------------------------------------------
-- Author: i0gan
-- Email : l418894113@gmail.com
-- Date  : 2022-12-04
-- Description: 打印table
-----------------------------------------------------------------------------

function print_t(table)
    local out_str = ""
    if table == nil then
        out_str = out_str .. "the table is nil\n"
        out_str = out_str .. debug.traceback()
        return out_str
    end
    
    local key = ""
    level =  1
    local indent = ""
    for i = 1, level do
    indent = indent.."  "
    end
    
    if key ~= "" then
        out_str = out_str .. indent..key.." ".."=".." ".."{\n"
    else
        out_str = out_str .. indent .. "{\n"
    end
    
    key = ""
    for k,v in pairs(table) do
    if type(v) == "table" then
        key = k
        out_str = out_str .. indent .. key .. " =\n"
        PrintTable(v, level + 1)
    else
        local content = string.format("%s%s = %s", indent .. "  ",tostring(k), tostring(v))
        out_str = out_str .. content..";\n"
        end
    end
    out_str = out_str .. indent .. "}\n"

    return out_str
end

function PrintTable(table)
    print(print_t(table))
end

