#pragma once

class IRenderable
{
public:
    virtual void Draw() = 0;
    virtual void PreUpdate() = 0;
    virtual void Update() = 0;
};
