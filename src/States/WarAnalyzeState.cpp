#include "WarAnalyzeState.h"

#include "../Data/Savegame.h"
#include "../Parser/Parser.h"

WarAnalyzeState::WarAnalyzeState(Application& app)	:	Basestate(app)
{
	bs::Transform t;
	bs::GameObject gobj(t);

	gobj.material.texture_id = 2;
	gobj.material.normal_id = 1;		//FOR TEXTURE MAPPING

	gobj.model_id = "flatplane";
	m_gameObjects.emplace_back(gobj);
	
}

WarAnalyzeState::~WarAnalyzeState()
{
	//Wait for job to finish to prevent errors
	while(jobSystem.backgroundJobs() != 0)	{	}
}

bool WarAnalyzeState::input(float dt)
{
	bs::asset_manager->loaded = true;

	ImGui::NewFrame();
	vInput = Input::getInput(dt);
	auto& io = ImGui::GetIO();

	m_player.getInput(vInput);

	return false;
}

void WarAnalyzeState::update(float dt)
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

void WarAnalyzeState::lateUpdate(Camera* cam)
{

}

void WarAnalyzeState::updateGUI() 
{
	static bool showwindow = false;
	static bool consolewindow = false;

	static Savegame save;

	static uint8_t indexTab = 1;
	static std::vector<int> imgIDs = { 1 }; 
	static bool loaded = false;

	auto& io = ImGui::GetIO();

	constexpr auto windowflag = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove;// | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar;

	if(ImGui::Begin("Big Screen", nullptr, windowflag))
	{
		ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
		ImGui::SetWindowSize(io.DisplaySize);

		if(ImGui::Button("Load"))
		{
			save = loadSavegame("testdata/test.v2");
			loaded = true;
		}
		
		ImGui::Separator();

		if(loaded)
		{
			const auto& wars = save.getWars();
			ImGui::Text("Wars: ");

			for(const auto& [name, war] : wars)
			{
				ImGui::TextWrapped("Name: %s, Winner: %s, Battles: %zu", 
					name.c_str(),
					(war.doesAttackerWin ? war.attackers.at(0).c_str() : war.defenders.at(0).c_str()),
					war.battles.size());
			}
		}
	}
	ImGui::End();
}

void WarAnalyzeState::render(Renderer* renderer)
{	
	for (auto& obj : m_gameObjects)
	{
		obj.getCurrentTransform();
		renderer->drawObject(obj);
	}
}

void WarAnalyzeState::createObject(bs::Transform& t, const std::string& name) 
{
	bs::GameObject gObj(t, name);
	m_gameObjects.emplace_back(gObj);
}

void WarAnalyzeState::removeObject(const std::string& name) 
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

PlayerController& WarAnalyzeState::getPlayer() 
{
	return m_player;
}
