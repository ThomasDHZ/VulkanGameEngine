local Player = {}

function Player:OnSpawn(entity, startPos)
    self.entity = entity
    self.speed = 300.0
    print("Player spawned at (" .. startPos.x .. ", " .. startPos.y .. ")")
end

function Player:OnUpdate(dt)
    local transform = GetTransform(self.entity)
    
    print("Player OnUpdate called with dt = " .. dt)
    
    -- Example movement
    if Input.IsKeyDown("D") then
        transform:Move(self.speed * dt, 0)
    end
end

return Player