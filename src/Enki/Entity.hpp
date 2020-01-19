#pragma once

//STD
#include <sstream>

//LIBS
//For whatever reason fmt and enet conflict a bit because of the ordering of winsock includes
//fmt includes all of WIN32, including winsocks2, so defining this fixes that
#define WIN32_LEAN_AND_MEAN
#include <spdlog/fmt/ostr.h>
#include <SFML/Graphics.hpp>

//SELF
#include "Enki/GameData.hpp"
#include "Enki/Hash.hpp"
#include "Enki/Managers/NetworkManager.hpp"
#include "Enki/Networking/Client.hpp"
#include "Enki/Networking/Packet.hpp"
#include "Enki/Renderer.hpp"

namespace enki
{
using EntityID = std::int64_t;

struct EntityInfo
{
	HashedID type{};
	std::string name{};

	EntityID ID{};
	ClientID ownerID{};
	EntityID parentID{};

	std::vector<EntityID> childIDs{};
};

inline bool operator==(const EntityInfo& lhs, const EntityInfo& rhs)
{
	return lhs.ID == rhs.ID &&
		   lhs.name == rhs.name &&
		   lhs.type == rhs.type &&
		   lhs.ownerID == rhs.ownerID &&
		   lhs.parentID == rhs.parentID &&
		   lhs.childIDs == rhs.childIDs;
}

inline bool operator!=(const EntityInfo& lhs, const EntityInfo& rhs)
{
	return !(lhs == rhs);
}

inline std::ostream& operator<<(std::ostream& os, const EntityInfo& info)
{
	std::stringstream ss;

	ss << "Type: " << info.type
	   << ", Name: " << info.name
	   << ", ID: " << info.ID
	   << ", ownerID: " << info.ownerID
	   << ", parentID: " << info.parentID
	   << ", children = {";

	for (auto id : info.childIDs)
	{
		ss << "\n"
		   << id;
	}

	if (info.childIDs.empty())
	{
		ss << "}";
	}
	else
	{
		ss << "\n}";
	}

	return os << ss.str();
}

inline Packet& operator<<(Packet& p, EntityInfo& e)
{
	p << e.ID
	  << e.ownerID
	  << e.type
	  << e.name
	  << e.parentID
	  << e.childIDs;
	return p;
}

inline Packet& operator>>(Packet& p, EntityInfo& e)
{
	p >> e.ID
	  >> e.ownerID
	  >> e.type
	  >> e.name
	  >> e.parentID
	  >> e.childIDs;
	return p;
}

class Entity
{
public:
	//Each derived entity will have info and game_data
	//passed to it by the scenetree when it is created
	Entity(EntityInfo info, GameData* game_data)
		: info(std::move(info))
		, game_data(game_data)
	{
	}

	virtual ~Entity() = default;

	//Called when an entity is created
	//Called for children before parents
	virtual void onSpawn([[maybe_unused]] Packet p){};

	virtual void onDespawn(){};

	//Called when an SFML event occurs
	virtual void input([[maybe_unused]] sf::Event& e){};

	//Called once each game loop
	virtual void update([[maybe_unused]] float dt){};

	//Called once each game loop
	virtual void draw([[maybe_unused]] Renderer* renderer){};

	//Called when a client connects so the full state can be sent
	virtual void serializeOnConnection(Packet& p) { serializeOnTick(p); }
	//Called when a client connects and we receive serialized data
	virtual void deserializeOnConnection(Packet& p) { deserializeOnTick(p); }

	//Called as often as the network_tick_rate of the entity
	virtual void serializeOnTick([[maybe_unused]] Packet& p) {}
	//Called when receiving an entity update from a network tick
	virtual void deserializeOnTick([[maybe_unused]] Packet& p) {}

	[[nodiscard]] inline bool isLocal() const
	{
		//todo: better than localFromID?
		return info.ID < 0;
	}

	//Determines if we have control over the entity
	//Always true for local entities
	[[nodiscard]] inline bool isOwner() const
	{
		if (isLocal())
		{
			return true;
		}
		else
		{
			//If the owner ID matches our ID then we own it
			return game_data->network_manager->client->getID() == info.ownerID;
		}
	}

	//Should not be modified directly in most cases
	//todo: fix access
	EntityInfo info;

	//Provides access to scenetree and network manager for all entities
	GameData* game_data;

	//Used by the Scenetree to mark entities for removal
	//todo: move out of entity, it's not safe to change it manually if networked
	bool remove = false;

	/*Every x ticks the entity will be automatically serialized via serializeOnTick
		The real time delay between ticks is dependant on the Network Manager's network_send_rate
		With a network_send_rate of 60 and a network_tick_rate of 1, this entity is serialized 60 times a second.
		With a network_tick_rate of 10, this entity is serialized 6 times a second.
		With a network_tick_rate of 0, this entity is never serialized automatically at regular intervals, just once on connection

		Using network_send_rate as part of a formula can allow for specifying the real-time delay between serialization
		e.g. network_tick_rate = network_send_rate / 10.0f, for roughly 10 serializations per second (depending on integer truncation).
		This relies on the network_send_rate not being modified after the network_tick_rate is set*/
	std::uint32_t network_tick_rate = 0;
};
}	// namespace enki