#pragma once

#include "Basestate.h"
#include "../Application.h"
#include "../Controllers/PlayerController.h"

#include "../Data/Savegame.h"

#include <Engine.h>

class WarAnalyzeState : public Basestate
{
public:
    WarAnalyzeState(Application& app);
    ~WarAnalyzeState();

	//Add an object to the gamestate list
	void createObject(bs::Transform& t, const std::string& name);
	//Remove object from the gamestate list
	void removeObject(const std::string& name);

    bool input(float dt) override;
    void update(float dt) override;
	
	void lateUpdate(Camera* cam) override;
	void render(Renderer* renderer) override;

	PlayerController& getPlayer() override;

private:
	void updateGUI();

	std::vector<bs::GameObject> m_gameObjects;
	PlayerController m_player;

	Input::Inputs vInput;

	Savegame save;

	uint8_t indexTab;
	bool loaded;

	std::string selectedWarName;
	int selectedBattle;
};
