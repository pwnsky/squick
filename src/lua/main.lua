Squick = nil;

Env = {
    path = '',
    module_mgr = {},
    app_id = -1,
    app_type = '',
    app_name = '',
}

function Require(file)
    require(file)
end

-- Main 函数调用，业务代码避免在此初始化，该函数调用优先级高于c++层的模块链接初始化。
function Main(context)
    Squick = context;
    Squick:LogInfo("Hello Lua, awake script_module " );
    Env.path = Squick:GetScriptPath()
    local path = Env.path
    print('path: ', path)
    package.path = path .. '/?.lua;'

    Squick:LogInfo("lua module execute");
    Env.app_id = Squick:AppID();
    Env.app_type= Squick:AppType();
    Env.app_name= Squick:AppName();
end


-- C++ 层的 Awake Start AfterStart 这三个函数在模块中请谨慎定义使用
function Awake()

end

function Start()

end

function AfterStart()
    
end

-- 在 ReadyUpdate 才进行初始化 Lua 模块，目的是让C++层的所有模块已链接完毕
function ReadyUpdate()
    Load()
    ModuleMgr:Start()
end

function Update()
    ModuleMgr:Update()
end

function BeforeDestroy()
    ModuleMgr:Destroy()
end

function Destroy()
    
end

function HotReload()
    Load()
end

function Load() 
    Require("common.init");
    Require("test.init");
    Require("proto.init");
    local node_init = {
        [ServerType.ST_GAME ] = function()
            Require("node.game.init");
        end,
        [ServerType.ST_LOBBY] = function ()
            Require("node.lobby.init");
        end
    }
    if(node_init[Env.app_type]) then
        node_init[Env.app_type]()
    else
        print("Not surppot this app type: " .. Env.app_type)
    end
end
