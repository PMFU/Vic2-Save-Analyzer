#include "WarAnalyzeState.h"

#include "../Parser/Parser.h"
#include "../Parser/Filepicker.h"

#include <fstream>

WarAnalyzeState::WarAnalyzeState(Application& app)	:	Basestate(app)
{
	indexTab = 1;
	loaded = false;
	selectedBattle = -1;
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
	auto& io = ImGui::GetIO();

	constexpr auto windowflag = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove;// | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar;
	constexpr auto tableflags = ImGuiTableFlags_Resizable;

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
						loaded = true;
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
					fileToLoad = "Selected Save File";
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

			if(ImGui::BeginTable("WarListTable", 4, tableflags))
			{
				ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
				ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_None);
				ImGui::TableSetupColumn("Start", ImGuiTableColumnFlags_None);
				ImGui::TableSetupColumn("War Goal", ImGuiTableColumnFlags_None);
				ImGui::TableSetupColumn("Battle Count", ImGuiTableColumnFlags_None);
				ImGui::TableHeadersRow();

				for(const auto& [name, war] : wars)
				{
					ImGui::TableNextRow();

					ImGui::TableSetColumnIndex(0);
					const ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;
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
						case 0:	//Name
						{
							//ImGui::Text(name.c_str());
							break;
						}
						case 1:	//Start
						{
							ImGui::Text("%s", war.start.getText().c_str());
							break;
						}
						case 2:	//War Goal
						{
							ImGui::Text("%s", war.wargoal.c_str());
							break;
						}
						case 3:	//Battle Count
						{
							//The war end results cannot be parsed, so doing battle count instead
							ImGui::Text("%lu", war.battles.size());
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

		if(ImGui::BeginChild("War, Battle Table Menu", {0.0f, 0.0f}, false, windowflag))
		{
			const auto& wars = save.getWars();
			if(selectedWarName.empty())	//If there is no selection
			{
				ImGui::EndChild();
				indexTab = 2;
				return;
			}

			ImGui::BeginGroup();	//Top Middle Menu

			ImGui::Text("Battles Fought in the War");

			ImGui::EndGroup();

			ImGui::BeginGroup();	//Middle Menu

			const auto& war = wars.at(selectedWarName);

			if(ImGui::BeginTable("BattleListTable", 6, tableflags/*, ImVec2(fifthwidth * 3, 0.0f)*/))
			{
				ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
				ImGui::TableSetupColumn("Location Name", ImGuiTableColumnFlags_None);
				ImGui::TableSetupColumn("Start[Not Accurate]", ImGuiTableColumnFlags_None);
				ImGui::TableSetupColumn("War Goal", ImGuiTableColumnFlags_None);
				ImGui::TableSetupColumn("Location", ImGuiTableColumnFlags_None);
				ImGui::TableSetupColumn("Casualties", ImGuiTableColumnFlags_None);
				ImGui::TableSetupColumn("Result", ImGuiTableColumnFlags_None);
				ImGui::TableHeadersRow();

				int battleIndex = 0;
				for(const auto& battle : war.battles)
				{
					ImGui::TableNextRow();

					ImGui::TableSetColumnIndex(0);
					const ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;
					bool selected = selectedBattle == battleIndex;
					const auto numberStr = "##" + std::to_string(battleIndex);
					if (ImGui::Selectable((battle.name + numberStr).c_str(), selected, selectable_flags, ImVec2(0.0f, 0.0f)))
                    {
						selectedBattle = battleIndex;
					}
                    for (int column = 0; column < 6; column++)
                    {
                        ImGui::TableSetColumnIndex(column);

						switch (column)
						{
						case 0:	//Name
						{
							//ImGui::Text(battle.name.c_str());
							break;
						}
						case 1:	//Start Date
						{
							ImGui::Text(war.start.getText().c_str());
							break;
						}
						case 2:	//War Goal
						{
							ImGui::Text("%s", war.wargoal.c_str());
							break;
						}
						case 3:	//Location (prov id)
						{
							const auto loc = std::to_string(battle.location);
							ImGui::Text("%s", loc.c_str());
							break;
						}
						case 4:	//Total Casualties
						{
							ImGui::Text("%lu", battle.atklosses + battle.deflosses);
							break;
						}
						case 5:	//Battle Count
						{
							ImGui::Text(battle.doesAttackerWin ? "Victory" : "Defeat");
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
				indexTab = 3;
				return;
			}

			const float width = ImGui::GetWindowWidth() / 2;
			const auto& battle = save.getWars().at(selectedWarName).battles[selectedBattle];

			//Top Description
			ImGui::SetWindowFontScale(2.0f);
			ImGui::Text("Battle of %s", battle.name.c_str());
			
			//String for total losses
			const auto losses = std::to_string(battle.atklosses + battle.deflosses);
			//String for "Attacker Won"/"Lost"
			const auto resultVictory = std::string("Attacker ").append((battle.doesAttackerWin) ? ("Won") : ("Lost"));
			//String for location
			const auto loc = std::to_string(battle.location);
			
			//String for date
			/*const Date d;
			const auto battleDate = d.getText();*/

			//Casualties in Battle
			ImGui::Text("Casualties:");
			ImGui::SameLine(400);
			ImGui::Text("%s", losses.c_str());

			//Date of Battle
			ImGui::Text("Date:");
			ImGui::SameLine(400);
			ImGui::Text("%s", "Not Found" /*battleDate.c_str()*/);
			
			//Locaiton
			ImGui::Text("Location:");
			ImGui::SameLine(400);
			ImGui::Text("%s", loc.c_str());
			
			//Result of Battle
			ImGui::Text("%s", resultVictory.c_str());
			
			ImGui::SetWindowFontScale(1.0f);
			ImGui::Separator();
			
			//Attacker Side
			ImGui::BeginGroup();

			ImGui::Text("Attacker");

			ImGui::Dummy(ImVec2(width, 12.0f));

			ImGui::Text("Country: %s", battle.countryATK.c_str());

			ImGui::Text("Leader: %s", battle.leaderATK.c_str());

			const auto atkLosses = std::to_string(battle.atklosses);
			ImGui::Text("Losses : %s", atkLosses.c_str());

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

			ImGui::Text("Country: %s", battle.countryDEF.c_str());

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
		
		// ImGui::Separator();

		if(loaded)
		{
			const auto& wars = save.getWars();
			// ImGui::Text("Wars: ");

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
