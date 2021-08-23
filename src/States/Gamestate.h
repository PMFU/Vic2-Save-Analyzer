#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "Basestate.h"

#include "../Application.h"
#include "../Controllers/PlayerController.h"

#include <Engine.h>

#include <atomic>

class GameState : public Basestate
{
public:
    GameState(Application& app);

	//Add an object to the gamestate list
	void createObject(bs::Transform& t, const std::string& name);
	//Remove object from the gamestate list
	void removeObject(const std::string& name);

    bool input(float dt) override;
    void update(float dt) override;
	
	void lateUpdate(Camera* cam) override;
	void render(Renderer* renderer) override;

	PlayerController& getPlayer() override;

    ~GameState() override;
protected:


private:
	void updateGUI();


	std::vector<bs::GameObject> m_gameObjects;
	PlayerController m_player;

	Input::Inputs vInput;

	bool hovering = false;
};

#endif // GAMESTATE_H
