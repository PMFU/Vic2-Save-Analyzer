#include "Menustate.h"


Menustate::Menustate(Application& app) : Basestate(app)
{
	
}

Menustate::~Menustate()
{

}

PlayerController& Menustate::getPlayer() 
{
	return m_player;
}

bool Menustate::input(float dt)
{
	ImGui::NewFrame();
	vInput = Input::getInput(dt);
	auto& io = ImGui::GetIO();
	

	return false;
}


void Menustate::update(float dt)
{
	static uint8_t menuIndex = 1;

	
	switch(menuIndex)
	{
		case 1:
		{
			
			break;
		}

		case 2:
		{
			
			break;
		}
	}
}

void Menustate::lateUpdate(Camera* cam)
{

}

//called in Application.cpp loop
void Menustate::render(Renderer* renderer)
{		
	for (auto& obj : m_gameObjects)
	{
		obj.getCurrentTransform();
		renderer->drawObject(obj);
	}
}

