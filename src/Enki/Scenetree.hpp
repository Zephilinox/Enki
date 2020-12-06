#pragma once

//STD
#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <vector>

//LIBS
#include <spdlog/spdlog.h>

//SELF
#include "Enki/Entity.hpp"
#include "Enki/Hash.hpp"
#include "Enki/Input/Events.hpp"
#include "Enki/Networking/RPC.hpp"
#include "Enki/Networking/RPCManager.hpp"
#include "Enki/Renderer/Renderer.hpp"

namespace enki
{
using EntityType = HashedID;

class Scenetree;

struct VersionEntityPair
{
	std::uint32_t version;
	std::unique_ptr<Entity> entity;
};

struct EntityChildCreationInfo
{
	EntityType type{};
	std::string name;
	Packet spawnInfo;
	std::vector<EntityChildCreationInfo> children{};
};

inline Packet& operator<<(Packet& p, EntityChildCreationInfo& e)
{
	p << e.type << e.name << e.spawnInfo << e.children;
	return p;
}

inline Packet& operator>>(Packet& p, EntityChildCreationInfo& e)
{
	p >> e.type >> e.name >> e.spawnInfo >> e.children;
	return p;
}

class Scenetree
{
public:
	struct ErrorCodeRemove
	{
		enum ErrorCode : std::uint8_t
		{
			Success,
			Unknown,
			IDWasZero,
			IDWasLocal,
			IDWasNotLocal,
			IndexOutOfBounds,
			IDDoesNotMatchFoundID,
		} errorCode = Unknown;

		constexpr ErrorCodeRemove(ErrorCode err)
			: errorCode(err){};

		constexpr operator bool() const
		{
			return errorCode == Success;
		}
	};

	using BuilderFunction = std::function<std::unique_ptr<Entity>(EntityInfo)>;

	Scenetree(NetworkManager* network_manager);
	void enableNetworking();
	NetworkManager* getNetworkManager() const;

	template <typename T>
	bool registerEntity(EntityType type, std::vector<EntityChildCreationInfo> children);

	template <typename T, typename... Args>
	bool registerEntity(EntityType type, std::vector<EntityChildCreationInfo> children, Args... args);

	[[nodiscard]] const std::vector<EntityID>& getRootEntitiesIDs() const
	{
		return entitiesParentless;
	}

	[[nodiscard]] std::size_t getEntityCount() const
	{
		return entities[Local].size() + entities[Networked].size();
	}

	void forEachEntity(std::function<void(const Entity&)> function);
	void forEachEntity(std::function<void(const Entity&)> function, std::vector<EntityID> ids);

	Entity* createEntityLocal(EntityType type,
		std::string name = "",
		EntityID parentID = 0,
		Packet spawnInfo = {},
		const std::vector<EntityChildCreationInfo>& children = {});

	void createEntityNetworkedRequest(EntityType type,
		std::string name = "",
		EntityID parentID = 0,
		Packet spawnInfo = {},
		const std::vector<EntityChildCreationInfo>& children = {});

	ErrorCodeRemove removeEntity(EntityID ID);
	ErrorCodeRemove removeEntityLocal(EntityID ID);

	Entity* findEntity(const EntityID ID) const;
	Entity* getEntityUnsafe(const EntityID ID);

	bool registerChildren(EntityType type, std::vector<EntityChildCreationInfo> children);

	std::vector<Entity*> getEntitiesFromRoot(EntityID ID = 0);

	void input(Event& e);
	void update(float dt);
	void draw(Renderer* renderer);

	/*Mark the entity for deletion next frame
	Will be sent across the network if not a local entity*/
	void deleteEntity(EntityID ID);

	//Vector will be empty if none found
	[[nodiscard]] std::vector<Entity*> findEntitiesByType(HashedID type) const;
	template <typename T>
	[[nodiscard]] std::vector<T*> findEntitiesByType(HashedID type) const;

	//Vector will be empty if none found
	[[nodiscard]] std::vector<Entity*> findEntitiesByName(const std::string& name) const;

	//Vector will be empty if none found
	[[nodiscard]] std::vector<Entity*> findEntitiesByOwner(ClientID owner) const;

	//Vector will be empty if none found
	[[nodiscard]] std::vector<Entity*> findEntitiesByParent(EntityID parent) const;

	//Vector will be empty if none found
	[[nodiscard]] std::vector<Entity*> findEntitiesByPredicate(const std::function<bool(const Entity&)>& predicate) const;

	/*nullptr if not found.
	Returns first entity found after static_cast to template type*/
	template <typename T = Entity>
	[[nodiscard]] T* findEntityByType(HashedID type) const;

	//nullptr if not found. Returns first entity found
	template <typename T = Entity>
	[[nodiscard]] T* findEntityByName(const std::string& name) const;

	//nullptr if not found. Returns first entity found
	template <typename T = Entity>
	[[nodiscard]] T* findEntityByPredicate(const std::function<bool(const Entity&)>& predicate) const;

	RPCManager rpc_man;
	
	//Used by the server when receiving a creation request, sends one packet with all the data
	void createEntityNetworkedFromRequest(EntityInfo info,
		const Packet& spawnInfo,
		const std::vector<EntityChildCreationInfo>& children);

private:

	//used by the above to fill out packet without sending more
	Entity* createEntityNetworkedFromRequestImpl(EntityInfo info,
		const Packet& spawnInfo,
		const std::vector<EntityChildCreationInfo>& children,
		Packet& p);

	//Used by clients when receiving a packet containing a tree of entities that they need to reproduce on their machine
	void createEntitiesFromTreePacket(Packet p);

	void input(Event& e, EntityID ID);
	void update(float dt, EntityID ID);
	void draw(Renderer* renderer, EntityID ID);

	//todo: I need a better name for this, help
	void fillEntitiesFromChildren(Entity* parent, std::vector<EntityID> children,
		std::vector<Entity*>& ents);

	bool checkChildrenValid(std::set<EntityType>& parentTypes,
		const std::vector<EntityChildCreationInfo>& children);

	//Networking
	void sendAllNetworkedEntitiesToClient(ClientID client_id);

	void receivedPacketFromClient(Packet p);
	void receivedEntityRPCFromClient(Packet& p);
	void receivedEntityDeletionFromClient(Packet& p);

	void receivedPacketFromServer(Packet p);
	void receivedEntityCreationFromServer(Packet p);
	void onNetworkTick();

	template <int tag>
	Entity* _findEntity(EntityID ID) const;
	
	Entity* _findEntity(EntityID ID) const;
	
	//////////////////

	std::shared_ptr<spdlog::logger> console;

	const unsigned int Local = 1;
	const unsigned int Networked = 0;

	std::array<std::vector<VersionEntityPair>, 2> entities;
	std::array<std::priority_queue<std::uint32_t, std::vector<std::uint32_t>, std::greater<>>, 2> entities_free_indices;

	std::map<EntityType, BuilderFunction> registeredTypes;
	std::map<EntityType, std::vector<EntityChildCreationInfo>> registeredChildCreationInfo;

	std::vector<EntityID> entitiesParentless;

	//Networking
	NetworkManager* network_manager;
	ManagedConnection mc1;
	ManagedConnection mc2;
	ManagedConnection mc3;

	bool network_ready = false;
	std::uint32_t total_network_ticks = 0;
};

template <typename T>
bool Scenetree::registerEntity(const EntityType type, std::vector<EntityChildCreationInfo> children)
{
	registeredTypes[type] = [](EntityInfo info) -> std::unique_ptr<Entity> {
		return std::make_unique<T>(std::move(info));
	};

	const bool valid = registerChildren(type, std::move(children));
	if (!valid)
	{
		registeredTypes[type] = {};
		return false;
	}

	return true;
}

template <typename T, typename... Args>
bool Scenetree::registerEntity(const EntityType type, std::vector<EntityChildCreationInfo> children, Args... args)
{
	registeredTypes[type] = [args...](EntityInfo info) -> std::unique_ptr<Entity> {
		return std::make_unique<T>(std::move(info), args...);
	};

	const bool valid = registerChildren(type, std::move(children));
	if (!valid)
	{
		registeredTypes[type] = {};
		return false;
	}

	return true;
}

template <typename T>
T* Scenetree::findEntityByType(HashedID type) const
{
	for (const auto& [version, entity] : entities[Local])
	{
		if (entity && entity->info.type == type)
			return static_cast<T*>(entity.get());
	}

	for (const auto& [version, entity] : entities[Networked])
	{
		if (entity && entity->info.type == type)
			return static_cast<T*>(entity.get());
	}

	return nullptr;
}

template <typename T>
T* Scenetree::findEntityByName(const std::string& name) const
{
	for (const auto& [version, entity] : entities[Local])
	{
		if (entity && entity->info.name == name)
			return static_cast<T*>(entity.get());
	}

	for (const auto& [version, entity] : entities[Networked])
	{
		if (entity && entity->info.name == name)
			return static_cast<T*>(entity.get());
	}

	return nullptr;
}

template <typename T>
T* Scenetree::findEntityByPredicate(const std::function<bool(const Entity&)>& predicate) const
{
	for (const auto& [version, entity] : entities[Local])
	{
		if (entity && predicate(*entity))
			return static_cast<T*>(entity.get());
	}

	for (const auto& [version, entity] : entities[Networked])
	{
		if (entity && predicate(*entity))
			return static_cast<T*>(entity.get());
	}

	return nullptr;
}

template <typename T>
std::vector<T*> Scenetree::findEntitiesByType(HashedID type) const
{
	std::vector<T*> ents;

	for (const auto& [version, entity] : entities[Local])
	{
		if (entity && entity->info.type == type)
			ents.push_back(static_cast<T*>(entity.get()));
	}

	for (const auto& [version, entity] : entities[Networked])
	{
		if (entity && entity->info.type == type)
			ents.push_back(static_cast<T*>(entity.get()));
	}

	return ents;
}

template <int tag>
Entity* Scenetree::_findEntity(EntityID ID) const
{
	if (ID == 0)
		return nullptr;

	const auto local = static_cast<bool>(ID >> 63);
	const auto version = static_cast<std::uint32_t>(ID >> 31);
	const auto index = static_cast<std::uint32_t>(ID & 0x0000'0000'7FFF'FFFFLL) - 1;

	if (index >= entities[local].size())
	{
		spdlog::get("Enki")->warn("tag {}. Entity {} has an index of {} which is beyond {}, the number of existing entities", tag, prettyID(ID), index, entities[local].size());
		return nullptr;
	}

	if (entities[local][index].version != version)
	{
		//todo: should we warn users when we fail to find an entity because of a different version?
		//tag 1 is internal find entity calls, e.g. as a result of calling scenetree->update(), so it ensures children/parent ids are valid
		if constexpr (tag == 1)
		{
			spdlog::get("Enki")->warn("tag {}. Entity {} found with version {}, but expected version {}", tag, prettyID(ID), entities[local][index].version, version);
		}
		return nullptr;
	}

	return entities[local][index].entity.get();
}

inline void printTree(Scenetree* tree, EntityID root = 0, const int depth = 0)
{
	if (tree == nullptr)
		return;

	if (root == 0)
	{
		std::cout << "Scenetree:\n";
		auto parentlessIDs = tree->getRootEntitiesIDs();
		for (auto id : parentlessIDs)
			printTree(tree, id, depth + 1);
	}
	else
	{
		const auto* e = tree->findEntity(root);
		if (e)
		{
			std::string prefix;
			prefix.reserve(8ULL * (static_cast<std::size_t>(depth) + 1ULL));
			
			for (int i = 0; i < depth - 1; ++i)
				prefix += "|       ";

			prefix += "\\_______";

			std::cout << prefix << fmt::format("{} {} {}\n",
				prettyHash(e->info.type),
				e->info.name,
				prettyID(e->info.ID));

			for (auto id : e->info.childIDs)
				printTree(tree, id, depth + 1);
		}
	}
	
}
}	 // namespace enki