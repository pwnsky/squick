Proto = {}

function Proto:Encode(rpc, t)
    if (Env.Debug) then
        PrintTable(t)
    end
    return Squick:Encode(rpc, t)
end


function Proto:Decode(rpc, d)
    local t = Squick:Decode(rpc, d)
    if (Env.Debug) then
        PrintTable(t)
    end
end