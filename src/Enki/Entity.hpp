#pragma once

//LIBS
//For whatever reason fmt and enet conflict a bit because of the ordering of winsock includes
//fmt includes all of WIN32, including winsocks2, so defining this fixes that
#define WIN32_LEAN_AND_MEAN
#include <spdlog/fmt/ostr.h>
#include <SFML/Graphics.hpp>

//SELF
#include "Networking/Packet.hpp"
#include "GameData.hpp"
#include "Networking/Client.hpp"
#include "Managers/NetworkManager.hpp"
#include "Renderer.hpp"

namespace enki
{
	using EntityID = std::int32_t;

	struct EntityInfo
	{
		//todo: replace strings with compile-time hashed strings for space saving at the cost of debugability
		std::string type = "";
		std::string name = "";

		EntityID ID = 0;
		ClientID ownerID = 0;
		EntityID parentID = 0;
	};

	inline bool operator ==(const EntityInfo& lhs, const EntityInfo& rhs)
	{
		return lhs.ID == rhs.ID &&
			lhs.name == rhs.name &&
			lhs.type == rhs.type &&
			lhs.ownerID == rhs.ownerID &&
			lhs.parentID == rhs.parentID;
	}

	inline std::ostream& operator <<(std::ostream& os, const EntityInfo& info)
	{
		return os << "Type: " << info.type << ", Name: " << info.name << ", ID: " << info.ID << ", ownerID: " << info.ownerID << ", parentID: " << info.parentID;
	}

	inline Packet& operator <<(Packet& p, EntityInfo& e)
	{
		p << e.ID
			<< e.ownerID
			<< e.type
			<< e.name
			<< e.parentID;
		return p;
	}

	inline Packet& operator >>(Packet& p, EntityInfo& e)
	{
		p >> e.ID
			>> e.ownerID
			>> e.type
			>> e.name
			>> e.parentID;
		return p;
	}

	class Entity
	{
	public:

		//Each derived entity will have info and game_data
		//passed to it by the scenegraph when it is created
		Entity(EntityInfo info, GameData* game_data)
			: info(info)
			, game_data(game_data)
		{
		}

		virtual ~Entity() = default;

		//Called when an entity is created
		//Called for children before parents
		virtual void onSpawn([[maybe_unused]]Packet& p) {};

		//Called when an SFML event occurs
		virtual void input([[maybe_unused]]sf::Event& e) {};

		//Called once each game loop
		virtual void update([[maybe_unused]]float dt) {};

		//Called once each game loop
		virtual void draw([[maybe_unused]]Renderer* renderer) {};

		//Called when a client connects so the full state can be sent
		virtual void serializeOnConnection(Packet& p) { serializeOnTick(p); }
		//Called when a client connects and we receive serialized data
		virtual void deserializeOnConnection(Packet& p) { deserializeOnTick(p); }

		//Called as often as the network_tick_rate of the entity
		virtual void serializeOnTick([[maybe_unused]]Packet& p) {}
		//Called when receiving an entity update from a network tick
		virtual void deserializeOnTick([[maybe_unused]]Packet& p) {}

		//Determines if we have control over the entity
		//Always true for local entities
		[[nodiscard]]
		inline bool isOwner() const
		{
			//ID less than 0 is local
			if (info.ID < 0)
			{
				return true;
			}

			//If we are the server and the owner isn't specified then we own it
			if (!game_data->network_manager->client)
			{
				return info.ownerID == 0;
			}

			//If the owner ID matches our ID then we own it
			return game_data->network_manager->client->getID() == info.ownerID;
		}

		//Should not be modified directly in most cases
		EntityInfo info;

		//Provides access to scenegraph and network manager for all entities
		GameData* game_data;

		//Used by the Scenegraph to mark entities for removal
		//Can be used by the owning entity for safely marking itself for deletion next frame
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
}