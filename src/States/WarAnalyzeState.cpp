#include "WarAnalyzeState.h"

#include "../Data/Savegame.h"
#include "../Parser/Parser.h"
#include "../Parser/Filepicker.h"

#include <fstream>

WarAnalyzeState::WarAnalyzeState(Application& app)	:	Basestate(app)
{
	/*
	bs::Transform t;
	bs::GameObject gobj(t);

	gobj.material.texture_id = 2;
	gobj.material.normal_id = 1;		//FOR TEXTURE MAPPING

	gobj.model_id = "flatplane";
	m_gameObjects.emplace_back(gobj);*/
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

	//m_player.getInput(vInput);

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
	static Savegame save;

	static uint8_t indexTab = 1;
	static bool loaded = false;

	static std::string selectedWarName;
	static int selectedBattle = -1;

	auto& io = ImGui::GetIO();

	constexpr auto windowflag = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove;// | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar;

	auto topmenu = [&]()
	{
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
	};

	auto saveselect = [&]()
	{
		static std::string fileToLoad = "testdata/test.v2";
		static std::string fileString;
		static bool fromFilepicker = false;

		if(ImGui::BeginChild("SaveSelect", {0.0f, 0.0f}, false, windowflag))
		{
			if(ImGui::Button("Load"))
			{
				if(!fromFilepicker)
				{
					if(fileToLoad.empty())
					{
						std::cout << "No file given\n";
					}
					else
					{
						std::ifstream testfile(fileToLoad);
						if(testfile.is_open())
						{
							std::cout << "File Exists. Loading now.\n";
							save = loadSavegame(fileToLoad);
							loaded = true;
						}
					}
				}
				else
				{
					if(!fileString.empty())
					{
						fromFilepicker = true;
						save = loadSavegameFromString(fileString);
					}
					else
					{
						fromFilepicker = false;
					}
				}
			}

			ImGui::Button(fileToLoad.c_str()); ImGui::SameLine();
			if(ImGui::SmallButton("..."))
			{
				fileString = openFilePickerFile();
				if(!fileString.empty())
				{
					fromFilepicker = true;
				}
				else
				{
					fromFilepicker = false;
				}
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
						selectedBattle = -1;
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

				int battleIndex = 0;
				for(const auto& battle : war.battles)
				{
					ImGui::TableNextRow();

					ImGui::TableSetColumnIndex(0);
					ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;
					bool selected = selectedBattle == battleIndex;
					if (ImGui::Selectable(battle.name.c_str(), selected, selectable_flags, ImVec2(0.0f, 0.0f)))
                    {
						selectedBattle = battleIndex;
					}
                    for (int column = 0; column < 6; column++)
                    {
                        ImGui::TableSetColumnIndex(column);

						switch (column)
						{
						case 0:
						{
							//ImGui::Text(battle.name.c_str());
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

					battleIndex += 1;
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
			if(selectedBattle < 0)
			{
				ImGui::EndChild();
				return;
			}

			const float width = ImGui::GetWindowWidth() / 2;
			const auto& battle = save.getWars().at(selectedWarName).battles[selectedBattle];

			//Top Description
			ImGui::SetWindowFontScale(2.0f);
			ImGui::Text("%s", battle.name.c_str());
			
			//String for total losses
			const auto losses = std::to_string(battle.atklosses + battle.deflosses);
			//String for "Attacker Won"/"Lost"
			const auto resultVictory = std::string("Attacker ").append((battle.doesAttackerWin) ? ("Won") : ("Lost"));
			//String for location
			const auto loc = std::to_string(battle.location);
			//String for date
			const Date d;
			const auto battleDate = d.getText();

			ImGui::Text("Casualties: %s\t\tResult: %s", losses.c_str(), resultVictory.c_str());
			ImGui::Text("Date: %s\t\tLocation: %s", battleDate.c_str(), loc.c_str());
			

			ImGui::SetWindowFontScale(1.0f);
			
			//Attacker Side
			ImGui::BeginGroup();

			ImGui::Text("Attacker");

			ImGui::Dummy(ImVec2(width, 12.0f));

			ImGui::Text("Country: %s", battle.countryATK);

			ImGui::Text("Leader: %s", battle.leaderATK.c_str());

			const auto atkLosses = std::to_string(battle.atklosses);
			ImGui::Text("Losses : %s", atkLosses);

			ImGui::Text("Units:");
			
			for(const auto& unit : battle.atkUnits)
			{
				const auto unitsize = std::to_string(unit.size);
				ImGui::Text("\t%s : %s", unit.name.c_str(), unitsize.c_str());
			}

			ImGui::EndGroup();
			ImGui::SameLine();
			//Defender Side
			ImGui::BeginGroup();

			ImGui::Text("Defender");

			ImGui::Dummy(ImVec2(width, 12.0f));

			ImGui::Text("Country: %s", battle.countryDEF);

			ImGui::Text("Leader: %s", battle.leaderDEF.c_str());

			const std::string defLosses = std::to_string(battle.deflosses);
			ImGui::Text("Losses : %s", defLosses.c_str());

			ImGui::Text("Units:");
			
			for(const auto& unit : battle.defUnits)
			{
				const auto unitsize = std::to_string(unit.size);
				ImGui::Text("\t%s : %s", unit.name.c_str(), unitsize.c_str());
			}

			ImGui::EndGroup();
		}
		ImGui::EndChild();
	};

	if(ImGui::Begin("Big Screen", nullptr, windowflag))
	{
		ImGui::SetWindowFontScale(2.0f);
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
