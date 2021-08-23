#include "Gamestate.h"


GameState::GameState(Application& app)	:	Basestate(app)
{
	bs::Transform t;
	bs::GameObject gobj(t);

	gobj.material.texture_id = 2;
	gobj.material.normal_id = 1;		//FOR TEXTURE MAPPING

	gobj.model_id = "flatplane";
	m_gameObjects.emplace_back(gobj);
	
}

GameState::~GameState()
{
	//Wait for job to finish to prevent errors
	while(jobSystem.backgroundJobs() != 0)	{	}
}

bool GameState::input(float dt)
{
	bs::asset_manager->loaded = true;

	ImGui::NewFrame();
	vInput = Input::getInput(dt);
	auto& io = ImGui::GetIO();

	m_player.getInput(vInput);

	return false;
}

void GameState::update(float dt)
{
	void* data[] = { &dt, &m_player, &m_gameObjects };

	// Player and World updates as a Job
	Job update = jobSystem.createJob([](Job job)
	{
		float deltatime = *static_cast<float*>(job.data[0]);

		static_cast<PlayerController*>(job.data[1])->update(deltatime);

	}, data);

	// Updates for all objects
	Job objUpdates = jobSystem.createJob([](Job job)
	{
		for (auto& obj : *static_cast<std::vector<bs::GameObject>*>(job.data[2]))
		{
			// A job for updating each individual object
			Job doObjectUpdate = jobSystem.createJob([&obj](Job jobObj)
				{
					obj.update();
				});
			jobSystem.schedule(doObjectUpdate);
		}
	}, data);


	/// Schedule the jobs
	jobSystem.schedule(update);

	jobSystem.wait();

	updateGUI();
}

void GameState::lateUpdate(Camera* cam)
{

}

void GameState::updateGUI() 
{
	static bool showwindow = false;
	static bool consolewindow = false;

	static uint8_t indexTab = 1;
	static std::vector<int> imgIDs = { 1 }; 

	auto& io = ImGui::GetIO();

	constexpr auto windowflag = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove;// | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar;

}

void GameState::render(Renderer* renderer)
{	
	for (auto& obj : m_gameObjects)
	{
		obj.getCurrentTransform();
		renderer->drawObject(obj);
	}
}

void GameState::createObject(bs::Transform& t, const std::string& name) 
{
	bs::GameObject gObj(t, name);
	m_gameObjects.emplace_back(gObj);
}

void GameState::removeObject(const std::string& name) 
{
	bool remove = false;
	m_gameObjects.erase(std::remove_if(m_gameObjects.begin(), m_gameObjects.end(), [&name, &remove](const bs::GameObject& elem) -> bool
		{
			remove = true;
			return elem.model_id == name;
		}), m_gameObjects.end());
	
	if (remove) 
	{
		std::cout << "Object: " << name << " deleted\n";
	}
	else 
	{
		std::cout << "Unable to delete object\n";
	}
}

PlayerController& GameState::getPlayer() 
{
	return m_player;
}
