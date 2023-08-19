function CreatEnumTable(tbl, index) 
    local enumtbl = {} 
    local enumindex = index or 0 
    for i, v in ipairs(tbl) do 
        enumtbl[v] = enumindex + i 
    end 
    return enumtbl 
end

ObjectEvent =
{
    "COE_CREATE_NODATA",
    "COE_CREATE_BEFORE_ATTACHDATA",
    "COE_CREATE_LOADDATA",
    "COE_CREATE_AFTER_ATTACHDATA",
    "COE_CREATE_BEFORE_EFFECT",
    "COE_CREATE_EFFECTDATA",
    "COE_CREATE_AFTER_EFFECT",
    "COE_CREATE_READY",
    "COE_CREATE_HASDATA",
    "COE_CREATE_FINISH",
    "COE_CREATE_CLIENT_FINISH",
    "COE_BEFOREDESTROY",
    "COE_DESTROY",
}
ObjectEvent = CreatEnumTable(ObjectEvent, -1)

RecordOpType =
{
    "Add",
    "Del",
    "AfterDel",
    "Swap",
    "Create",
    "Update",
    "Cleared",
    "Sort",
    "Cover",
    "UNKNOW",
}

RecordOpType = CreatEnumTable(RecordOpType, -1)

ServerType =
{
    "ST_NONE",
    "SQUICK_ST_REDIS",
    "SQUICK_ST_MYSQL",
    "ST_MASTER",
    "ST_LOGIN",
    "ST_PROXY",
    "ST_GAME",
    "ST_WORLD",
    "SQUICK_ST_DB",
    "ST_MAX",
    "SQUICK_ST_GATEWAY",
};
ServerType = CreatEnumTable(ServerType, -1)
