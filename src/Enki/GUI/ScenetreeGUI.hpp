#pragma once

//SELF
#include "Enki/Scenetree.hpp"
#include <Enki/GUI/IMGUI/imgui.h> //todo: move out

namespace enki
{

class ScenetreeGUI : public Entity
{
public:
	ScenetreeGUI(EntityInfo info, Scenetree* scenetree);

	void input(Event& e) override;
	void update(float dt) override;

	bool show = true;

private:
	Scenetree* scenetree;
};

}