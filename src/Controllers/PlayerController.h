#pragma once

#include <Engine.h>
#include "../Camera.h"

class PlayerController
{
public:
    PlayerController();

    void update(float deltatime);

    void getInput(Input::Inputs inputs);

    Camera& getCurrentCamera();

    bs::Transform& getTransform();

    short country = 0;

private:
	Camera debugCamera;
	Camera gameCamera;

    // set 0 for debugcamera, set 1 for gamecamera
    int currentCamera = 1;
    bool fixedMapFlag = false;
    bs::Transform transform;
    
    bs::vec3 velocity;
};
