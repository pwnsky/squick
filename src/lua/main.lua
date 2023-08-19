Squick = nil;

Env = {
    path = '',
    module_mgr = {},
    app_id = -1,
    app_type = '',
    app_name = '',
}

-- init_script_system 函数调用，业务代码避免在此初始化，该函数调用优先级高于c++层的模块链接初始化。
function Main(context)
    Squick = context;
    Squick:LogInfo("Hello Lua, awake script_module, " .. tostring(Squick));
    Env.path = Squick:GetScriptPath()
    local path = Env.path
    print('path: ', path)
    package.path = path .. '/?.lua;'
    .. path .. '/lib/json/?.lua;'
    .. path .. '/config/?.lua;'
    .. path .. '/server/?.lua;'
    .. path .. '/common/?.lua;'
    require("common.init");

    Squick:LogInfo("lua module execute");
    Env.app_id = Squick:AppID();
    Env.app_type= Squick:AppType();
    Env.app_name= Squick:AppName();
    
    local node_init = {
        [ServerType.ST_GAME ] = function()
            require("game.init");
        end,
    }
    if(node_init[Env.app_type]) then
        node_init[Env.app_type]()
    else
        print("Not surppot this app type: " .. Env.app_type)
    end
end

---------------------------------------------
-- module_awake module_init module_after_init 这三个函数在模块中请谨慎定义使用
function Awake(...)

end

function Init(...)

end

function AfterInit(...)

end

---------------------------------------------
-- 在 ready_update 才进行初始化 lua 模块，目的是让c++层的所有模块已链接完毕，避免lua层调用c++层出现模块引用错误
function ReadyUpdate(...)

end

function BeforeDestroy(...)

end

function Destroy(...)

end