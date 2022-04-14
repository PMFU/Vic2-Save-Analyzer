#pragma once

class Controller 
{
public:
    Controller();

    virtual void update(float deltatime);

    virtual ~Controller() = default;
private:
    
};