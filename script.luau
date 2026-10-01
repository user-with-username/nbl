local undertaker = lookup(16, "Undertaker")

function tick()
    for i = 0, (server.playersCount - 1) do
        local p = server:getClientInfo(i)
        local char = server.objectManager:getObject(p.objectId)

        if char ~= nil and char.data == undertaker then
            p.ultiCharge = p.maxUltiCharge
        end
    end
end