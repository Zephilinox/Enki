#pragma once

//STD
#include <charconv>
#include <sstream>

//LIBS
//For whatever reason fmt and enet conflict a bit because of the ordering of winsock includes
//fmt includes all of WIN32, including winsocks2, so defining this fixes that
#define WIN32_LEAN_AND_MEAN
#include <spdlog/fmt/ostr.h>
#include <SFML/Graphics.hpp>

//SELF
#include "Enki/Hash.hpp"
#include "Enki/Input/Events.hpp"
#include "Enki/Managers/NetworkManager.hpp"
#include "Enki/Networking/Client.hpp"
#include "Enki/Networking/Packet.hpp"
#include "Enki/Renderer.hpp"
#include "Enki/Messages/Message.hpp"

namespace enki
{
using EntityID = std::int64_t;

constexpr EntityID generateEntityID(bool local, std::uint32_t version, std::uint32_t index)
{
	//technically there's a small issue with index when it goes over 31 bits (2,147,483,647)
	//since it's just ignored, it esentially rolls over back to zero, so that's a check we should make
	//however if every entity is 100 bytes, then 2,147,483,647 entities is 215 gigabytes. I think we're safe for now.

	//local, version, index
	return static_cast<EntityID>(local) << 63 |
		   static_cast<EntityID>(version) << 31 |
		   ((static_cast<EntityID>(index) & 0x0000'0000'7FFF'FFFFLL) + 1);	//mask out top bit, which is bottom bit of version
																			//+1 because an ID of 0 is !local, version 0, index 0, and ID 0 needs to be invalid, so force index to start at 1
}

inline EntityID generateEntityIDFromPrettyID(std::string_view prettyID)
{
	if (prettyID == "none")
		return 0;

	//todo: error checking and shit
	auto posL = prettyID.find_first_of('L');
	auto posV = prettyID.find_first_of('V');
	auto posI = prettyID.find_first_of('I');
	auto subL = prettyID.substr(posL + 1, posV - posL);
	auto subV = prettyID.substr(posV + 1, posI - posV);
	auto subI = prettyID.substr(posI + 1);

	int local;
	std::uint32_t version;
	std::uint32_t index;
	auto success = std::from_chars(subL.data(), subL.data() + subL.size(), local);
	success = std::from_chars(subV.data(), subV.data() + subV.size(), version);
	success = std::from_chars(subI.data(), subI.data() + subI.size(), index);

	return generateEntityID(static_cast<bool>(local), version, index);
};

struct IDComponentLocalVersionIndex
{
	bool local;
	std::uint32_t version;
	std::uint32_t index;
};

constexpr IDComponentLocalVersionIndex splitID(EntityID ID)
{
	//local, version, index
	return {
		static_cast<bool>(ID >> 63),
		static_cast<std::uint32_t>(ID >> 31),
		static_cast<std::uint32_t>(ID & 0x0000'0000'7FFF'FFFFLL) - 1, //mask out top bit, which is bottom bit of version
																			//subtract 1 for the same reason as above, just reversed, so we get the correct index
	};
}

constexpr bool localFromID(EntityID ID)
{
	return static_cast<bool>(ID >> 63);
}

constexpr std::uint32_t versionFromID(EntityID ID)
{
	return static_cast<std::uint32_t>(ID >> 31);
}

constexpr std::uint32_t indexFromID(EntityID ID)
{
	return static_cast<std::uint32_t>(ID & 0x0000'0000'7FFF'FFFFLL) - 1;	//mask out top bit, which is bottom bit of version
																			//subtract 1 for the same reason as above, just reversed, so we get the correct index
}

inline std::string prettyID(EntityID ID)
{
	if (ID == 0)
		return "none";

	//todo: look in to compile time formatting using MPark.Format
	return fmt::format("L{}V{}I{}",
		static_cast<int>(localFromID(ID)),
		versionFromID(ID),
		indexFromID(ID));
}

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
	const auto main_match = lhs.ID == rhs.ID &&
					  lhs.name == rhs.name &&
					  lhs.type == rhs.type &&
					  lhs.ownerID == rhs.ownerID &&
					  lhs.parentID == rhs.parentID;
	
	const auto children_match = lhs.childIDs == rhs.childIDs;
	//todo: because a networked entity can have a local child, the children might not match. we could ensure we update the childids of networked entities, but then those id's are invalid when trying to look them up
	//worry aobut it later
	return main_match;
}

inline bool operator!=(const EntityInfo& lhs, const EntityInfo& rhs)
{
	return !(lhs == rhs);
}

inline std::ostream& operator<<(std::ostream& os, const EntityInfo& info)
{
	std::stringstream ss;

#if !defined(ENKI_RUNTIME_HASH_FAST) || defined(ENKI_HASH_DEBUG)
	ss << "Type: " << hashToString(info.type)
#else
	ss << "Type: " << info.type
#endif
	   << ", Name: " << info.name
	   << ", ID: " << prettyID(info.ID)
	   << ", ownerID: " << info.ownerID
	   << ", parentID: " << prettyID(info.parentID);
	
	if (!info.childIDs.empty())
	{
		ss << ", children = {";

		for (auto id : info.childIDs)
		{
			ss << "\n"
			   << prettyID(id);
		}

		ss << "}";
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
	p >> e.ID >> e.ownerID >> e.type >> e.name >> e.parentID >> e.childIDs;
	return p;
}

class Entity
{
public:
	static constexpr EntityID InvalidID = 0;

	//Each derived entity will have info
	//passed to it by the scenetree when it is created
	Entity(EntityInfo info)
		: info(std::move(info))
	{
	}

	//The scenetree double-buffers. The previous frame is drawn, allowing the current frame to process at the same time.
	//In addition, the current frame update will use entity information from the previous frame, so it becomes deterministic and can also be threaded
	//If you're not using the scene tree then you might not care about this, but I don't want to provide a default returning nullptr because it can be surprising
	virtual std::unique_ptr<Entity> clone() = 0;
	
	virtual ~Entity() = default;

	//Called when an entity is created
	//Called for children before parents
	virtual void onSpawn([[maybe_unused]] Packet p){};

	virtual void onDespawn(){};

	//Called when an SFML event occurs
	virtual void input([[maybe_unused]] Event& e){};

	//Called once each game loop
	virtual void update([[maybe_unused]] float dt){};

	//Called once each game loop
	virtual void draw([[maybe_unused]] Renderer* renderer){};

	virtual void receive([[maybe_unused]] Message* msg) { assert(false); throw;};
	
	//Called when a client connects so the full state can be sent
	virtual void serializeOnConnection(Packet& p) { serializeOnTick(p); }
	//Called when a client connects and we receive serialized data
	virtual void deserializeOnConnection(Packet& p) { deserializeOnTick(p); }

	//Called as often as the network_tick_rate of the entity
	virtual void serializeOnTick([[maybe_unused]] Packet& p) {}
	//Called when receiving an entity update from a network tick
	virtual void deserializeOnTick([[maybe_unused]] Packet& p) {}

	[[nodiscard]] inline bool isNetworked() const
	{
		//todo: better than localFromID?
		return info.ID > 0;
	}

	[[nodiscard]] inline bool isLocal() const
	{
		//todo: better than localFromID?
		return info.ID < 0;
	}

	//Determines if we have control over the entity
	//Always true for local entities
	//If nullptr is passed and it's not local, it will always be false
	[[nodiscard]] inline bool isOwner(NetworkManager* network_manager) const
	{
		if (isLocal())
			return true;
		else if (network_manager)
			return network_manager->client->getID() == info.ownerID;
		else
			return false;
	}

	//Should not be modified directly in most cases
	//todo: fix access
	EntityInfo info;

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
}	 // namespace enki