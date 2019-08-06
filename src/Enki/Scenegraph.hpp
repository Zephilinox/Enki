#pragma once

//STD
#include <vector>
#include <map>
#include <experimental/map>
#include <functional>

//LIBS
#include <spdlog/spdlog.h>

//SELF
#include "Enki/Entity.hpp"
#include "Enki/GameData.hpp"
#include "Enki/Networking/RPC.hpp"
#include "Enki/Networking/RPCManager.hpp"
#include "Enki/Renderer.hpp"
#include "Enki/Hash.hpp"

namespace enki
{
	//Used when registering which child entity's an entity will have
	struct ChildEntityCreationInfo
	{
		HashedID type;
		std::string name;
		Packet spawnInfo;
	};

	class Scenegraph
	{
	public:
		using BuilderFunction = std::function<std::unique_ptr<Entity>(EntityInfo)>;

		//game_data will be passed to all created entities
		Scenegraph(GameData* game_data);

		/*Set up the scenegraph for networked entities
		Make sure the server or client has already been started*/
		void enableNetworking();

		//Calls input on all entities in the scenegraph
		void input(sf::Event& e);

		//Calls update on all entities in the scenegraph
		void update(float dt);

		//Calls draw on all entities in the scenegraph
		void draw(Renderer* renderer);

		/*Register an entity for construction at a later date using that entity's type
		Use this when you want more control over the entity's construction
		E.g. wanting to use std::move for an entity constructor parameter*/
		void registerEntity(HashedID type, BuilderFunction builder);

		//Register an entity for construction at a later date using that entity's type
		//Each additional parameter is passed by value to that entity's constructor
		template <typename T, typename... Args>
		void registerEntity(HashedID type, Args... args);

		/*Register any number of entities as children of the specified entity
		Each parameter after the first should be a ChildEntityCreationInfo struct*/
		template <typename... Args>
		void registerEntityChildren(HashedID type, Args... args);

		/*Create an entity with the given entity info.
		Requires entity name and type
		Cannot create a networked entity this way unless you are the server.*/
		Entity* createEntity(EntityInfo info);

		/*Create an entity with the given entity info.
		Requires entity name and type
		The packet of data will be passed to onSpawn() of the entity
		Cannot create a networked entity this way unless you are the server.*/
		Entity* createEntity(EntityInfo info, Packet& spawnInfo);

		/*Create an entity with the given entity info.
		Requires entity name and type
		If you are the server the entity is created and sent to all clients
		If you are the client an entity creation request is sent to the server
		and you will be the owner*/
		void createNetworkedEntity(EntityInfo info);

		/*Create an entity with the given entity info.
		Requires entity name and type
		The packet of data will be passed to onSpawn() of the entity
		If you are the server the entity is created and sent to all clients
		If you are the client an entity creation request is sent to the server
		and you will be the owner*/
		void createNetworkedEntity(EntityInfo info, Packet& spawnInfo);

		//Get a pointer based on its ID
		//Will throw exception if not found
		[[nodiscard]]
		Entity* getEntity(EntityID entityID) const;

		//Make sure an entity exists before trying to get it
		[[nodiscard]]
		bool entityExists(EntityID entityID) const;

		/*Mark the entity for deletion next frame
		Will be sent across the network if not a local entity*/
		void deleteEntity(EntityID entityID);

		//Vector will be empty if none found
		[[nodiscard]]
		std::vector<Entity*> findEntitiesByType(HashedID type) const;

		//Vector will be empty if none found
		[[nodiscard]]
		std::vector<Entity*> findEntitiesByName(const std::string& name) const;

		//Vector will be empty if none found
		[[nodiscard]]
		std::vector<Entity*> findEntitiesByOwner(ClientID owner) const;

		//Vector will be empty if none found
		[[nodiscard]]
		std::vector<Entity*> findEntitiesByParent(EntityID parent) const;

		//Vector will be empty if none found
		[[nodiscard]]
		std::vector<Entity*> findEntitiesByPredicate(const std::function<bool(const Entity&)>& predicate) const;

		/*nullptr if not found.
		Returns first entity found after static_cast to template type*/
		template <typename T = Entity>
		[[nodiscard]]
		T* findEntityByType(HashedID type) const;

		//nullptr if not found. Returns first entity found
		template <typename T = Entity>
		[[nodiscard]]
		T* findEntityByName(const std::string& name) const;

		//nullptr if not found. Returns first entity found
		template <typename T = Entity>
		[[nodiscard]]
		T* findEntityByPredicate(const std::function<bool(const Entity&)>& predicate) const;

		RPCManager rpc_man;

	private:
		//this is all hacky bullshit because of how badly designed this scene tree is
		//REALLY FUCKING SLOW
		//that being said I don't want to touch it so yeah
		void inputHierarchy(sf::Event& e, EntityID parentID);
		void updateHierarchy(float dt, EntityID parentID);
		void drawHierarchy(enki::Renderer* renderer, EntityID parentID);

		void sendAllNetworkedEntitiesToClient(ClientID client_id);

		std::map<EntityID, std::unique_ptr<Entity>> entities;
		std::map<HashedID, std::vector<ChildEntityCreationInfo>> entities_child_types;
		std::map<HashedID, BuilderFunction> builders;

		EntityID ID = 1;
		EntityID localID = -1;
		GameData* game_data;

		ManagedConnection mc1;
		ManagedConnection mc2;
		ManagedConnection mc3;

		bool network_ready = false;
		std::shared_ptr<spdlog::logger> console;
		std::uint32_t total_network_ticks = 0;
	};

	template <typename T, typename... Args>
	void Scenegraph::registerEntity(HashedID type, Args... args)
	{
		//capture any additional constructor arguments by value because it's a safe default
		//users can use pointers for big objects
		builders[type] = [=](EntityInfo info)
		{
			return std::make_unique<T>(info, this->game_data, args...);
		};
	}

	template <typename... Args>
	void Scenegraph::registerEntityChildren(HashedID type, Args... args)
	{
		//We want to store args here if each child entity is valid
		//but rather than check it before we check it after and then delete

		//This is because we take advantage of list initialization for a vector
		//using a parameter pack to make it simpler to do the iteration after-the-fact

		entities_child_types[type] = { args... };

		for (const auto& child_type : entities_child_types[type])
		{
			if (!builders.count(child_type.type))
			{
				entities_child_types[type] = {};
				console->error("The entity {} is a child of {} but the child entity has not been registered.",
					child_type.type, type);
			}
		}

		if (entities_child_types[type].empty())
		{
			entities_child_types.erase(type);
		}
	}

	template <typename T>
	T* Scenegraph::findEntityByType(HashedID type) const
	{
		for (const auto& ent : entities)
		{
			if (ent.second->info.type == type)
			{
				return static_cast<T*>(ent.second.get());
			}
		}

		return nullptr;
	}

	template <typename T>
	T* Scenegraph::findEntityByName(const std::string& name) const
	{
		for (const auto& ent : entities)
		{
			if (ent.second->info.name == name)
			{
				return static_cast<T*>(ent.second.get());
			}
		}

		return nullptr;
	}

	template <typename T>
	T* Scenegraph::findEntityByPredicate(const std::function<bool(const Entity&)>& predicate) const
	{
		for (const auto& ent : entities)
		{
			if (predicate(*ent.second.get()))
			{
				return static_cast<T*>(ent.second.get());
			}
		}

		return nullptr;
	}
}