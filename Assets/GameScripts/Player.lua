local Player = {}

function Player:OnSpawn(entity, startPos, startRot)
    self.entity = entity
    self.speed = 300.0
    
    Print("=== PLAYER ONSPAWN CALLED ===")
    Print("Position: (" .. startPos.x .. ", " .. startPos.y .. ")")
end

function Player:OnUpdate(dt)
    Print("=== PLAYER ONUPDATE dt=" .. dt .. " ===")
    
    local transform = GetTransform(self.entity)
    
    if Input.IsKeyDown("D") then
        transform:Move(self.speed * dt, 0)
    end
    if Input.IsKeyDown("A") then
        transform:Move(-self.speed * dt, 0)
    end
end

return Player