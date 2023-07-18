#include "EcoAnalyzeState.h"

#include "../Parser/Parser.h"
#include "../Parser/Filepicker.h"

#include <fstream>

EcoAnalyzeState::EcoAnalyzeState(Application& app)	:	Basestate(app)
{
	indexTab = 1;
	loaded = false;
	// selectedBattle = -1;
}

EcoAnalyzeState::~EcoAnalyzeState()
{
	//Wait for job to finish to prevent errors
	while(jobSystem.backgroundJobs() != 0)	{	}
}

bool EcoAnalyzeState::input(float dt)
{
	bs::asset_manager->loaded = true;

	ImGui::NewFrame();
	vInput = Input::getInput(dt);
	auto& io = ImGui::GetIO();

	//m_player.getInput(vInput);

	return false;
}

void EcoAnalyzeState::update(float dt)
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

void EcoAnalyzeState::lateUpdate(Camera* cam)
{

}

void EcoAnalyzeState::updateGUI() 
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

			if(ImGui::Button("Eco") && loaded)
			{
				indexTab = 2;
			}
			ImGui::SameLine();

			/*if(ImGui::Button("War") && loaded)
			{
				indexTab = 3;
			}
			ImGui::SameLine();

			if(ImGui::Button("Battle") && loaded)
			{
				indexTab = 4;
			}*/
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

	auto eco = [&]()
	{
		/*if(ImGui::BeginChild("WarList", {0.0f, 0.0f}, false, windowflag))
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
		ImGui::EndChild();*/
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
			eco();
			break;
		default:
			indexTab = 1;
			break;
		}

		/*if(loaded)
		{
			const auto& wars = save.getWars();
			// ImGui::Text("Wars: ");

			for(const auto& [name, war] : wars)
			{
				ImGui::Text("Name: %s, Winner: %s, Battles: %zu", 
					name.c_str(),
					(war.doesAttackerWin ? war.attackers.at(0).c_str() : war.defenders.at(0).c_str()),
					war.battles.size());
			}
		}*/
	}
	ImGui::End();
}

void EcoAnalyzeState::render(Renderer* renderer)
{	
	for (auto& obj : m_gameObjects)
	{
		obj.getCurrentTransform();
		renderer->drawObject(obj);
	}
}

void EcoAnalyzeState::createObject(bs::Transform& t, const std::string& name) 
{
	bs::GameObject gObj(t, name);
	m_gameObjects.emplace_back(gObj);
}

void EcoAnalyzeState::removeObject(const std::string& name) 
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

PlayerController& EcoAnalyzeState::getPlayer() 
{
	return m_player;
}
