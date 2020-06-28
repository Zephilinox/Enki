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
#include "Enki/Renderer.hpp"
#include "Enki/Messages/Message.hpp"

namespace enki
{
using EntityType = HashedID;

class Scenetree;

struct VersionEntityPair
{
	VersionEntityPair(std::uint32_t version, std::unique_ptr<Entity> entity)
		: version(version)
		, entity(std::move(entity))
	{

	}

	VersionEntityPair(const VersionEntityPair& vep)
		: version(vep.version)
		, entity(vep.entity ? vep.entity->clone() : nullptr)
	{
	}
	
	VersionEntityPair(VersionEntityPair&& vep) noexcept
		: version(vep.version)
		, entity(std::move(vep.entity))
	{
	}
	
	VersionEntityPair& operator=(const VersionEntityPair& vep)
	{
		if (&vep != this)
		{
			version = vep.version;
			entity = vep.entity ? vep.entity->clone() : nullptr;
		}
		
		return *this;
	}

	VersionEntityPair& operator=(VersionEntityPair&& vep) noexcept
	{
		if (&vep != this)
		{
			version = vep.version;
			entity = std::move(vep.entity);
		}

		return *this;
	}
	
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
	p << e.type
	  << e.name
	  << e.spawnInfo
	  << e.children;
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
	static constexpr auto local = 1;
	static constexpr auto networked = 0;
	
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

	template <typename T>
	bool registerEntity(EntityType type, std::vector<EntityChildCreationInfo> children);

	template <typename T, typename... Args>
	bool registerEntity(EntityType type, std::vector<EntityChildCreationInfo> children, Args... args);

	[[nodiscard]] const std::vector<EntityID>& getRootEntitiesIDs() const
	{
		return frame_wip.entities_parentless;
	}

	[[nodiscard]] std::size_t getEntityCount() const
	{
		return frame_wip.entities[local].size() + frame_wip.entities[networked].size() - frame_wip.entities_free_indices[local].size() - frame_wip.entities_free_indices[networked].size();
	}

	void forEachEntity(std::function<void(const Entity&)> function);
	void forEachEntity(std::function<void(const Entity&)> function,
		const std::vector<EntityID> ids);

	Entity* createEntityLocal(const EntityType type,
		std::string name = "",
		const EntityID parentID = 0,
		Packet spawnInfo = {},
		const std::vector<EntityChildCreationInfo>& children = {});

	void createEntityNetworkedRequest(const EntityType type,
		std::string name = "",
		const EntityID parentID = 0,
		Packet spawnInfo = {},
		const std::vector<EntityChildCreationInfo>& children = {});

	ErrorCodeRemove removeEntity(const EntityID ID);
	ErrorCodeRemove removeEntityLocal(const EntityID ID);

	Entity* findEntity(const EntityID ID);
	Entity* getEntityUnsafe(const EntityID ID);

	bool registerChildren(const EntityType type, std::vector<EntityChildCreationInfo> children);

	std::vector<Entity*> getEntitiesFromRoot(EntityID ID = 0);

	void input(Event& e);
	void processMessages();
	void update(float dt);
	void draw(Renderer* renderer);

	void sendMessage(EntityID id, std::unique_ptr<Message> msg);
	
	/*Mark the entity for deletion next frame
		Will be sent across the network if not a local entity*/
	void deleteEntity(EntityID ID);

	//Vector will be empty if none found
	[[nodiscard]] std::vector<Entity*> findEntitiesByType(HashedID type) const;

	//Vector will be empty if none found
	[[nodiscard]] std::vector<Entity*> findEntitiesByName(const std::string& name) const;

	//Vector will be empty if none found
	[[nodiscard]] std::vector<Entity*> findEntitiesByOwner(ClientID owner) const;

	//Vector will be empty if none found
	[[nodiscard]] std::vector<Entity*> findEntitiesByParent(EntityID parent) const;

	//Vector will be empty if none found
	[[nodiscard]] std::vector<Entity*> findEntitiesByPredicate(
		const std::function<bool(const Entity&)>& predicate) const;

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

	//todo: was private, basically need to support a way for the user that is a server to request an entity is made with an owner that isn't the server
	//Used by the server when receiving a creation request, sends one packet with all the data
	void createEntityNetworkedFromRequest(EntityInfo info,
		const Packet& spawnInfo,
		const std::vector<EntityChildCreationInfo>& children);

private:
	class MessageEntityWrapper : public MessageID<hash_constexpr("EntityWrapper")>
	{
	public:
		MessageEntityWrapper(EntityID entity_id, std::unique_ptr<Message> msg)
			: entity_id(entity_id)
			, msg(std::move(msg))
		{
			
		}

		EntityID entity_id;
		std::unique_ptr<Message> msg;
	};
	
	struct Frame
	{
		std::vector<EntityID> entities_parentless;

		std::array<std::vector<VersionEntityPair>, 2> entities;
		std::array<std::priority_queue<std::uint32_t, std::vector<std::uint32_t>, std::greater<>>, 2> entities_free_indices;

		std::vector<Entity*> getEntitiesFromRoot(EntityID ID = 0);
		Entity* findEntity(const EntityID ID);

		//todo: I need a better name for this, help
		void fillEntitiesFromChildren(std::vector<EntityID> children,
			std::vector<Entity*>& ents);
	};

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

	//////////////////

	std::shared_ptr<spdlog::logger> console;

	std::map<EntityType, BuilderFunction> registeredTypes;
	std::map<EntityType, std::vector<EntityChildCreationInfo>> registeredChildCreationInfo;

	Frame frame_finished;
	Frame frame_wip;
	std::vector<std::unique_ptr<MessageEntityWrapper>> messages_for_next_frame;
	std::int64_t frame_count = 0;
	std::unordered_map<EntityID, std::pair<std::int64_t, EntityInfo>> dead_entity_history;

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

	bool valid = registerChildren(type, std::move(children));
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

	bool valid = registerChildren(type, std::move(children));
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
	for (const auto& [version, entity] : frame_wip.entities[local])
	{
		if (entity && entity->info.type == type)
		{
			return static_cast<T*>(entity.get());
		}
	}

	for (const auto& [version, entity] : frame_wip.entities[networked])
	{
		if (entity && entity->info.type == type)
		{
			return static_cast<T*>(entity.get());
		}
	}

	return nullptr;
}

template <typename T>
T* Scenetree::findEntityByName(const std::string& name) const
{
	for (const auto& [version, entity] : frame_wip.entities[local])
	{
		if (entity && entity->info.name == name)
		{
			return static_cast<T*>(entity.get());
		}
	}

	for (const auto& [version, entity] : frame_wip.entities[networked])
	{
		if (entity && entity->info.name == name)
		{
			return static_cast<T*>(entity.get());
		}
	}

	return nullptr;
}

template <typename T>
T* Scenetree::findEntityByPredicate(const std::function<bool(const Entity&)>& predicate) const
{
	for (const auto& [version, entity] : frame_wip.entities[local])
	{
		if (entity && predicate(*entity))
		{
			return static_cast<T*>(entity.get());
		}
	}

	for (const auto& [version, entity] : frame_wip.entities[networked])
	{
		if (entity && predicate(*entity))
		{
			return static_cast<T*>(entity.get());
		}
	}

	return nullptr;
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
		{
			printTree(tree, id, depth + 1);
		}
	}
	else
	{
		auto e = tree->findEntity(root);
		if (e)
		{
			std::string prefix;
			prefix.reserve(8ULL * (static_cast<std::size_t>(depth) + 1ULL));
						
			for (int i = 0; i < depth - 1; ++i)
			{
				prefix += "|       ";
			}

			prefix += "\\_______";

			#if !defined(ENKI_RUNTIME_HASH_FAST) || defined(ENKI_HASH_DEBUG)
				std::cout << prefix << fmt::format("{} {} {}\n",
				hashToString(e->info.type),
					e->info.name,
					prettyID(e->info.ID));
			#else
				std::cout << prefix << fmt::format("{} {} {}\n",
					e->info.type,
					e->info.name,
					prettyID(e->info.ID));
			#endif

			for (auto id : e->info.childIDs)
			{
				printTree(tree, id, depth + 1);
			}
		}
	}
}
}	 // namespace enki