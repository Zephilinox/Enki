#pragma once

//SELF
#include "CustomData.hpp"

//LIBS
#include <Enki/Entity.hpp>
#include <Enki/Signals/Signal.hpp>

class CollisionManager : public enki::Entity
{
public:
	CollisionManager(enki::EntityInfo info, CustomData* custom_data);

	void onSpawn(enki::Packet p) final;

	void update(float dt) final;
	void draw(enki::Renderer* renderer) final;

	void serializeOnTick(enki::Packet& p) final;
	void deserializeOnTick(enki::Packet& p) final;

private:
	CustomData* const custom_data;
};