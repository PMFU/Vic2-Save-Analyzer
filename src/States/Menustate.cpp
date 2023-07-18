#include "Menustate.h"

#include "WarAnalyzeState.h"
#include "EcoAnalyzeState.h"

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
	constexpr auto windowflag = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar;

	if(ImGui::Begin("Mode Selector", nullptr, windowflag))
	{
		//ImGui::WindowS

		if(ImGui::Button("War Analyzer"))
		{
			app.pushState(std::make_unique<WarAnalyzeState>(WarAnalyzeState(app)));
		}

		if(ImGui::Button("Eco Analyzer"))
		{
			app.pushState(std::make_unique<EcoAnalyzeState>(EcoAnalyzeState(app)));
		}
	}

	ImGui::End();

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

