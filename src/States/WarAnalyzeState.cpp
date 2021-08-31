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

	static std::string selectedWarName;
	static int selectedBattle = 0;

	auto& io = ImGui::GetIO();

	constexpr auto windowflag = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove;// | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar;

	auto topmenu = [&]()
	{
		//ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, ImVec2(0.0f, 8.0f));
		if(ImGui::BeginChild("TopBar", ImVec2(0.0f, 100.0f), false, windowflag))
		{
			if(ImGui::Button("Save Select"))
			{
				indexTab = 1;
			}
			ImGui::SameLine();

			if(ImGui::Button("Wars List") && loaded)
			{
				indexTab = 2;
			}
			ImGui::SameLine();

			if(ImGui::Button("War") && loaded)
			{
				indexTab = 3;
			}
			ImGui::SameLine();

			if(ImGui::Button("Battle") && loaded)
			{
				indexTab = 4;
			}
		}
		
		ImGui::EndChild();
		//ImGui::PopStyleVar();
	};

	auto saveselect = [&]()
	{
		if(ImGui::BeginChild("SaveSelect", {0.0f, 0.0f}, false, windowflag))
		{
			if(ImGui::Button("Load"))
			{
				// save = loadSavegame("testdata/test.v2");
				save = loadSavegame("testdata/Testsave.v2");
				loaded = true;
			}
		}
		ImGui::EndChild();
	};

	auto warslist = [&]()
	{
		if(ImGui::BeginChild("WarList", {0.0f, 0.0f}, false, windowflag))
		{
			const auto& wars = save.getWars();
			constexpr auto tableflags = ImGuiTableFlags_Reorderable | ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable;

			if(ImGui::BeginTable("WarListTable", 4, tableflags))
			{
				ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
				ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_None);
				ImGui::TableSetupColumn("Start", ImGuiTableColumnFlags_None);
				ImGui::TableSetupColumn("End", ImGuiTableColumnFlags_None);
				ImGui::TableSetupColumn("Result", ImGuiTableColumnFlags_None);
				ImGui::TableHeadersRow();

				for(const auto& [name, war] : wars)
				{
					ImGui::TableNextRow();

					ImGui::TableSetColumnIndex(0);
					ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;
					bool selected = selectedWarName == name;
					if (ImGui::Selectable(name.c_str(), selected, selectable_flags, ImVec2(0.0f, 0.0f)))
                    {
						selectedWarName = name;
					}
                    for (int column = 0; column < 4; column++)
                    {
                        ImGui::TableSetColumnIndex(column);

						switch (column)
						{
						case 0:
						{
							//ImGui::Text(name.c_str());
							break;
						}
						case 1:
						{
							ImGui::Text(war.start.getText().c_str());
							break;
						}
						case 2:
						{
							ImGui::Text("End");
							break;
						}
						case 3:
						{
							ImGui::Text(war.doesAttackerWin ? "Victory" : "Defeat");
							break;
						}	
						default:
							break;
						}
                    }
				}
				

				ImGui::EndTable();
			}
		}
		ImGui::EndChild();
	};

	auto war = [&]()
	{
		const float fifthwidth = io.DisplaySize.x * 0.2f;

		if(ImGui::BeginChild("War", {0.0f, 0.0f}, false, windowflag))
		{
			const auto& wars = save.getWars();
			if(selectedWarName.empty())	//If there is no selection
			{
				ImGui::EndChild();
				return;
			}

			ImGui::BeginGroup();	//Left Side Menu
			ImGui::PushItemWidth(fifthwidth);

			ImGui::Text("War Menu");

			
			ImGui::PopItemWidth();
			ImGui::EndGroup();
			ImGui::SameLine();

			ImGui::BeginGroup();	//Middle Menu

			const auto& war = wars.at(selectedWarName);

			ImGui::PushItemWidth(fifthwidth * 3); //make sure this is adjusted by the frame size

			if(ImGui::BeginTable("BattleListTable", 6, 0, ImVec2(fifthwidth * 3, 0.0f)))
			{
				ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
				ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_None);
				ImGui::TableSetupColumn("Start", ImGuiTableColumnFlags_None);
				ImGui::TableSetupColumn("End", ImGuiTableColumnFlags_None);
				ImGui::TableSetupColumn("Location", ImGuiTableColumnFlags_None);
				ImGui::TableSetupColumn("Casualties", ImGuiTableColumnFlags_None);
				ImGui::TableSetupColumn("Result", ImGuiTableColumnFlags_None);
				ImGui::TableHeadersRow();

				for(const auto& battle : war.battles)
				{
					ImGui::TableNextRow();
                    for (int column = 0; column < 6; column++)
                    {
                        ImGui::TableSetColumnIndex(column);

						switch (column)
						{
						case 0:
						{
							ImGui::Text(battle.name.c_str());
							break;
						}
						case 1:
						{
							ImGui::Text(war.start.getText().c_str());
							break;
						}
						case 2:
						{
							ImGui::Text("End");
							break;
						}
						case 3:
						{
							ImGui::Text("%h", battle.location);
							break;
						}
						case 4:
						{
							ImGui::Text("%lu", battle.atklosses + battle.deflosses);
							break;
						}
						case 5:
						{
							ImGui::Text(war.doesAttackerWin ? "Victory" : "Defeat");
							break;
						}	
						default:
							break;
						}
                    }
				}

				ImGui::EndTable();
			}

			ImGui::EndGroup();
			ImGui::PopItemWidth();
			ImGui::SameLine();
			
			ImGui::BeginGroup();	//Right Side Menu
			ImGui::PushItemWidth(fifthwidth);
			

			ImGui::PopItemWidth();
			ImGui::EndGroup();
			ImGui::SameLine();
		}
		ImGui::EndChild();
	};

	auto battle = [&]()
	{
		if(ImGui::BeginChild("Battle", {0.0f, 0.0f}, false, windowflag))
		{

		}
		ImGui::EndChild();
	};

	if(ImGui::Begin("Big Screen", nullptr, windowflag))
	{
		ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
		ImGui::SetWindowSize(io.DisplaySize);

		topmenu();

		ImGui::Separator();
		

		switch (indexTab)
		{
		case 1:
			saveselect();
			break;
		
		case 2:
			warslist();
			break;

		case 3:
			war();
			break;

		case 4:
			battle();
			break;
		
		default:
			break;
		}
		
		ImGui::Separator();

		if(loaded)
		{
			const auto& wars = save.getWars();
			ImGui::Text("Wars: ");

			/*for(const auto& [name, war] : wars)
			{
				ImGui::Text("Name: %s, Winner: %s, Battles: %zu", 
					name.c_str(),
					(war.doesAttackerWin ? war.attackers.at(0).c_str() : war.defenders.at(0).c_str()),
					war.battles.size());
			}*/
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
