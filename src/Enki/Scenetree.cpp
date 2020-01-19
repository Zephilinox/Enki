#include "Scenetree.hpp"

namespace enki
{
Scenetree::Scenetree(GameData* game_data)
	: rpc_man(game_data->network_manager)
	, game_data(game_data)
{
	if (game_data == nullptr)
	{
		throw;
	}

	console = spdlog::get("Enki");
	if (console == nullptr)
	{
		console = spdlog::stdout_color_mt("Enki");
	}
}

void Scenetree::enableNetworking()
{
	if (network_ready)
	{
		console->error(
			"Tried to enable networking for scenetree "
			"but networking is already enabled");
		return;
	}

	if (!game_data->network_manager->server &&
		!game_data->network_manager->client)
	{
		console->error(
			"Tried to enable networking for scenetree "
			"but neither the server nor the client have been started");
		return;
	}

	network_ready = true;

	if (game_data->network_manager->server)
	{
		mc1 = game_data->network_manager->server->on_packet_received.connect(
			this, &Scenetree::receivedPacketFromClient);
	}

	if (game_data->network_manager->client)
	{
		mc2 = game_data->network_manager->client->on_packet_received.connect(
			this, &Scenetree::receivedPacketFromServer);

		mc3 = game_data->network_manager->on_network_tick.connect(
			this, &Scenetree::onNetworkTick);
	}
}

void Scenetree::forEachEntity(std::function<void(const Entity&)> function)
{
	for (const auto& [version, ent] : entitiesLocal)
	{
		if (ent)
		{
			function(*ent);
		}
	}

	for (const auto& [version, ent] : entitiesNetworked)
	{
		if (ent)
		{
			function(*ent);
		}
	}
}

void Scenetree::forEachEntity(std::function<void(const Entity&)> function,
	const std::vector<EntityID> ids)
{
	std::vector<Entity*> ents;
	for (auto id : ids)
	{
		auto e = findEntity(id);
		if (e)
		{
			ents.push_back(e);
		}
	}

	for (auto e : ents)
	{
		function(*e);
	}
}

Entity* Scenetree::createEntityLocal(const EntityType type,
	std::string name,
	const EntityID parentID,
	Packet spawnInfo,
	const std::vector<EntityChildCreationInfo>& children)
{
	if (parentID != 0 && findEntity(parentID) == nullptr)
		return nullptr;
	if (registeredTypes[type] == nullptr)
		return nullptr;

	Entity* entity = nullptr;

	//no free indices available, create a new entity with version 0
	if (freeIndicesLocal.empty())
	{
		EntityInfo info;
		info.type = type;
		info.name = std::move(name);
		info.ID = generateEntityID(true, 0, entitiesLocal.size());
		info.parentID = parentID;

		if (parentID == 0)
		{
			entitiesParentless.push_back(info.ID);
		}

		auto e = registeredTypes[type](std::move(info), game_data);
		entity = e.get();

		entitiesLocal.emplace_back(VersionEntityPair{0, std::move(e)});
	}
	else	//reuse existing index
	{
		std::uint32_t index = freeIndicesLocal.top();
		freeIndicesLocal.pop();

		EntityInfo info;
		info.type = type;
		info.name = std::move(name);
		//version was incremented when the last entity here was deleted, no need to change it here
		info.ID = generateEntityID(true, entitiesLocal[index].version, index);
		info.parentID = parentID;

		if (parentID == 0)
		{
			entitiesParentless.push_back(info.ID);
		}

		auto e = registeredTypes[type](std::move(info), game_data);
		entity = e.get();

		entitiesLocal[index].entity = std::move(e);
	}

	for (const auto& child : registeredChildCreationInfo[type])
	{
		auto c = createEntityLocal(child.type, child.name, entity->info.ID, child.spawnInfo, child.children);
		entity->info.childIDs.push_back(c->info.ID);
	}

	for (const auto& child : children)
	{
		//todo: all of this copying is expensive, maybe we can move it out instead of taking a const ref?
		//not sure how well that would fit in to the usecases of passing spawninfo on construction, we'll see
		auto c = createEntityLocal(child.type, child.name, entity->info.ID, child.spawnInfo, child.children);
		entity->info.childIDs.push_back(c->info.ID);
	}

	entity->onSpawn(std::move(spawnInfo));

	return entity;
}

void Scenetree::createEntityNetworkedRequest(const EntityType type, std::string name, const EntityID parentID, Packet spawnInfo, const std::vector<EntityChildCreationInfo>& children)
{
	auto net_man = game_data->network_manager;
	if (!network_ready)
	{
		console->error(
			"Tried to request a networked entity "
			"when scenetree isn't network ready");
		return;
	}

	if (!net_man->client)
	{
		console->error(
			"Tried to request a networked entity "
			"when there is no client");
		return;
	}

	if (name.empty() || type == 0)
	{
		console->error(
			"Invalid info when creating networked entity."
			"\nname: {}, type: {}",
			name,
			type);
		return;
	}

	if (!registeredTypes.count(type))
	{
		console->error(
			"Tried to create entity without "
			"registering it first.\ntype: {}",
			type);
		return;
	}

	Packet p({PacketType::ENTITY_CREATION_REQUEST});
	p << type
	  << name
	  << parentID
	  << spawnInfo
	  << children;

	console->info(
		"Sending networked entity request as the client "
		"to the server.\n\t{}",
		EntityInfo{type, name, 0, 0, parentID});
	net_man->client->sendPacket(0, &p);
}

Scenetree::ErrorCodeRemove Scenetree::removeEntity(const EntityID ID)
{
	if (localFromID(ID))
	{
		return removeEntityLocal(ID);
	}

	return ErrorCodeRemove::Unknown;
}

Scenetree::ErrorCodeRemove Scenetree::removeEntityLocal(const EntityID ID)
{
	if (ID == 0)
		return ErrorCodeRemove::IDWasZero;

	const auto [local, version, index] = splitID(ID);

	if (!local)
		return ErrorCodeRemove::IDWasNotLocal;
	if (index >= entitiesLocal.size())
		return ErrorCodeRemove::IndexOutOfBounds;

	auto& e = entitiesLocal[index];
	if (ID != e.entity->info.ID)
		return ErrorCodeRemove::IDDoesNotMatchFoundID;
	//this should never happen, the version we store and the version in the ID has diverged
	if (version != e.version)
		std::abort();

	e.entity->onDespawn();

	for (auto c : e.entity->info.childIDs)
	{
		removeEntityLocal(c);
	}

	e.version++;
	e.entity.reset(nullptr);
	freeIndicesLocal.push(index);
	return ErrorCodeRemove::Success;
}

Entity* Scenetree::findEntity(const EntityID ID)
{
	if (ID == 0)
		return nullptr;

	const auto [local, version, index] = splitID(ID);

	if (local && index < entitiesLocal.size() && entitiesLocal[index].version == version)
	{
		return entitiesLocal[index].entity.get();
	}
	else if (!local && index < entitiesNetworked.size() && entitiesNetworked[index].version == version)
	{
		return entitiesNetworked[index].entity.get();
	}
	else
	{
		return nullptr;
	}
}

Entity* Scenetree::getEntityUnsafe(const EntityID ID)
{
	if (ID < 0)
	{
		return entitiesLocal[indexFromID(ID)].entity.get();
	}
	else
	{
		return entitiesNetworked[indexFromID(ID)].entity.get();
	}
}

bool Scenetree::registerChildren(const EntityType type, std::vector<EntityChildCreationInfo> children)
{
	if (!children.empty())
	{
		std::set<EntityType> parentTypes{type};

		if (!checkChildrenValid(parentTypes, children))
		{
			return false;
		}

		registeredChildCreationInfo[type] = children;	//do we need to do this beforehand?
	}

	return true;
}

std::vector<Entity*> Scenetree::getEntitiesFromRoot(EntityID ID)
{
	std::vector<Entity*> ents;

	if (ID == 0)
	{
		fillEntitiesFromChildren(entitiesParentless, ents);
	}
	else
	{
		auto e = findEntity(ID);
		if (e)
		{
			fillEntitiesFromChildren(e->info.childIDs, ents);
		}
	}

	return ents;
}

void Scenetree::input(Event& event)
{
	for (unsigned int i = 0; i < entitiesParentless.size(); ++i)
	{
		input(event, entitiesParentless[i]);
	}
}

void Scenetree::update(float dt)
{
	auto entities = getEntitiesFromRoot();
	std::vector<Entity*> entitiesToRemove;

	//todo: does not handle deleting children of deleted entities
	//mark as deleted automatically when doing the deletion?
	//todo: handle removing child from parent child ids

	//from parents to children
	for (auto e : entities)
	{
		if (e->remove)
		{
			auto [local, version, index] = splitID(e->info.ID);

			if (local)
			{
				entitiesLocal[index].entity->onDespawn();
			}
			else
			{
				entitiesNetworked[index].entity->onDespawn();
			}

			entitiesToRemove.push_back(e);
		}
	}

	//from children to parents
	for (auto it = entitiesToRemove.rbegin(); it != entitiesToRemove.rend(); ++it)
	{
		auto e = *it;
		auto [local, version, index] = splitID(e->info.ID);

		std::experimental::erase_if(entitiesParentless, [removedID = e->info.ID](EntityID id) {
			return id == removedID;
		});

		if (local)
		{
			entitiesLocal[index].version++;
			entitiesLocal[index].entity = nullptr;
		}
		else
		{
			entitiesNetworked[index].version++;
			entitiesNetworked[index].entity = nullptr;
		}
	}

	for (int i = 0; i < entitiesParentless.size(); ++i)
	{
		update(dt, entitiesParentless[i]);
	}
}

void Scenetree::draw(Renderer* renderer)
{
	for (int i = 0; i < entitiesParentless.size(); ++i)
	{
		draw(renderer, entitiesParentless[i]);
	}
}

void Scenetree::deleteEntity(EntityID ID)
{
	auto e = findEntity(ID);

	if (!e)
	{
		console->error(
			"Tried to delete entity {} "
			"but it doesn't exist",
			ID);
		return;
	}

	if (e->info.ID < 0)
	{
		e->remove = true;
	}
	else if (network_ready)
	{
		Packet p({ENTITY_DELETION});
		p << ID;

		if (game_data->network_manager->server)
		{
			game_data->network_manager->server->sendPacketToAllClients(0, &p);
		}
		else if (game_data->network_manager->client)
		{
			game_data->network_manager->client->sendPacket(0, &p);
		}
	}
}

std::vector<Entity*> Scenetree::findEntitiesByType(HashedID type) const
{
	std::vector<Entity*> ents;

	for (const auto& [version, entity] : entitiesLocal)
	{
		if (entity->info.type == type)
		{
			ents.push_back(entity.get());
		}
	}

	for (const auto& [version, entity] : entitiesNetworked)
	{
		if (entity->info.type == type)
		{
			ents.push_back(entity.get());
		}
	}

	return ents;
}

std::vector<Entity*> Scenetree::findEntitiesByName(const std::string& name) const
{
	std::vector<Entity*> ents;

	for (const auto& [version, entity] : entitiesLocal)
	{
		if (entity && entity->info.name == name)
		{
			ents.push_back(entity.get());
		}
	}

	for (const auto& [version, entity] : entitiesNetworked)
	{
		if (entity && entity->info.name == name)
		{
			ents.push_back(entity.get());
		}
	}

	return ents;
}

std::vector<Entity*> Scenetree::findEntitiesByOwner(ClientID owner) const
{
	std::vector<Entity*> ents;

	for (const auto& [version, entity] : entitiesLocal)
	{
		if (entity && entity->info.ownerID == owner)
		{
			ents.push_back(entity.get());
		}
	}

	for (const auto& [version, entity] : entitiesNetworked)
	{
		if (entity && entity->info.ownerID == owner)
		{
			ents.push_back(entity.get());
		}
	}

	return ents;
}

std::vector<Entity*> Scenetree::findEntitiesByParent(EntityID parent) const
{
	std::vector<Entity*> ents;

	for (const auto& [version, entity] : entitiesLocal)
	{
		if (entity && entity->info.parentID == parent)
		{
			ents.push_back(entity.get());
		}
	}

	for (const auto& [version, entity] : entitiesNetworked)
	{
		if (entity && entity->info.parentID == parent)
		{
			ents.push_back(entity.get());
		}
	}

	return ents;
}

std::vector<Entity*> Scenetree::findEntitiesByPredicate(const std::function<bool(const Entity&)>& predicate) const
{
	std::vector<Entity*> ents;

	for (const auto& [version, entity] : entitiesLocal)
	{
		if (entity && predicate(*entity))
		{
			ents.push_back(entity.get());
		}
	}

	for (const auto& [version, entity] : entitiesNetworked)
	{
		if (entity && predicate(*entity))
		{
			ents.push_back(entity.get());
		}
	}

	return ents;
}

////////////////////////
/////////////////PRIVATE
////////////////////////

void Scenetree::createEntityNetworkedFromRequest(EntityInfo info,
	const Packet& spawnInfo,
	const std::vector<EntityChildCreationInfo>& children)
{
	Packet p({PacketType::ENTITY_CREATION_TREE});
	auto e = createEntityNetworkedFromRequestImpl(std::move(info), spawnInfo, children, p);

	console->info(
		"Creating networked entity as the server "
		"to all clients. Sending tree\n\t{}",
		e->info);
	game_data->network_manager->server->sendPacketToAllExceptOneClient(1, 0, &p);
}

Entity* Scenetree::createEntityNetworkedFromRequestImpl(EntityInfo info,
	const Packet& spawnInfo,
	const std::vector<EntityChildCreationInfo>& children,
	Packet& p)
{
	if (info.name.empty() || info.type == 0)
	{
		console->error(
			"Invalid info when creating networked entity."
			"\n\t{}",
			info);
		throw;
	}

	if (!registeredTypes.count(info.type))
	{
		console->error(
			"Tried to create networked entity without "
			"registering it first.\n\t{}",
			info);
		throw;
	}

	if (info.ID < 0)
	{
		console->error(
			"Tried to create networked entity that has "
			"a local ID.\n\t{}",
			info);
		throw;
	}

	if (info.ID > 0)
	{
		console->error(
			"Tried to create networked entity that has "
			"an existing ID.\n\t{}",
			info);
		throw;
	}

	if (info.parentID != 0 && findEntity(info.parentID) == nullptr)
	{
		console->error(
			"Tried to create networked entity that has "
			"a non-existent parent.\n\t{}",
			info);
		throw;
	}

	std::uint32_t index;
	std::uint32_t version;

	if (freeIndicesNetworked.empty())
	{
		index = entitiesNetworked.size();
		version = 0;
		entitiesNetworked.emplace_back(VersionEntityPair{0, nullptr});
	}
	else
	{
		index = freeIndicesLocal.top();
		freeIndicesLocal.pop();
		version = entitiesNetworked[index].version;
	}

	info.ID = generateEntityID(false, version, index);

	if (info.parentID == 0)
	{
		entitiesParentless.push_back(info.ID);
	}

	Entity* entity;

	{
		auto e = registeredTypes[info.type](info, game_data);
		entity = e.get();
		entitiesNetworked[index].version = version;
		entitiesNetworked[index].entity = std::move(e);
	}

	for (auto& child : registeredChildCreationInfo[info.type])
	{
		auto c = createEntityNetworkedFromRequestImpl(
			{child.type, child.name, 0, info.ownerID, info.ID},
			child.spawnInfo,
			child.children,
			p);
		entity->info.childIDs.push_back(c->info.ID);
		p << c->info << child.spawnInfo;
	}

	for (auto& child : children)
	{
		auto c = createEntityNetworkedFromRequestImpl(
			{child.type, child.name, 0, info.ownerID, info.ID},
			child.spawnInfo,
			child.children,
			p);
		entity->info.childIDs.push_back(c->info.ID);
		p << c->info;
		p << child.spawnInfo;
	}

	entity->onSpawn(spawnInfo);
	return entity;
}

void Scenetree::createEntitiesFromTreePacket(Packet p)
{
	if (!game_data->network_manager->client)
	{
		throw;
	}

	try
	{
		auto info = p.read<EntityInfo>();
		auto spawnInfo = p.read<Packet>();

		auto [local, version, index] = splitID(info.ID);
		auto type = info.type;

		if (info.parentID == 0)
		{
			entitiesParentless.push_back(info.ID);
		}

		auto e = registeredTypes[type](std::move(info), game_data);
		auto entity = e.get();

		if (index >= entitiesNetworked.size())	//need to create a new one
		{
			while (index != entitiesNetworked.size())	//need to create empty ents to match required index
			{
				entitiesNetworked.emplace_back(VersionEntityPair{0, nullptr});
			}

			//now index == size, so last push back is the correct index
			entitiesNetworked.emplace_back(VersionEntityPair{version, std::move(e)});
		}
		else
		{
			if (entitiesNetworked[index].version != version)
			{
				throw;
			}
			else if (entitiesNetworked[index].entity != nullptr)
			{
				throw;
			}

			entitiesNetworked[index] = {version, std::move(e)};
		}

		//todo: this is calling it from parent-to-child, should be child-to-parent
		entity->onSpawn(spawnInfo);
	}
	catch (...)
	{
		//todo: this is a shit way of handling this problem
	}
}

void Scenetree::input(Event& event, EntityID ID)
{
	auto e = findEntity(ID);
	if (e)
	{
		e->input(event);
		for (auto childID : e->info.childIDs)
		{
			input(event, childID);
		}
	}
}

void Scenetree::update(float dt, EntityID ID)
{
	auto e = findEntity(ID);
	if (e)
	{
		e->update(dt);
		for (auto childID : e->info.childIDs)
		{
			update(dt, childID);
		}
	}
}

void Scenetree::draw(Renderer* renderer, EntityID ID)
{
	auto e = findEntity(ID);
	if (e)
	{
		e->draw(renderer);
		for (auto childID : e->info.childIDs)
		{
			draw(renderer, childID);
		}
	}
}

void Scenetree::fillEntitiesFromChildren(std::vector<EntityID> children, std::vector<Entity*>& ents)
{
	for (auto id : children)
	{
		auto e = findEntity(id);
		if (e)
		{
			ents.push_back(e);
			fillEntitiesFromChildren(e->info.childIDs, ents);
		}
	}
}

bool Scenetree::checkChildrenValid(std::set<EntityType>& parentTypes, const std::vector<EntityChildCreationInfo>& children)
{
	for (const auto& child : children)
	{
		//make sure none of the children match the parental hierarchy
		if (parentTypes.count(child.type))
			return false;

		//check registered grandchildren against our parental hierarchy and ourselves
		if (!registeredChildCreationInfo[child.type].empty())
		{
			parentTypes.insert(child.type);
			bool valid = checkChildrenValid(parentTypes, registeredChildCreationInfo[child.type]);
			parentTypes.erase(child.type);
			if (!valid)
				return false;
		}

		//check unregistered grandchildren against our parental hierarchy and ourselves
		if (!child.children.empty())
		{
			parentTypes.insert(child.type);
			bool valid = checkChildrenValid(parentTypes, child.children);
			parentTypes.erase(child.type);
			if (!valid)
				return false;
		}
	}

	return true;
}


void Scenetree::sendAllNetworkedEntitiesToClient(ClientID client_id)
{
	if (!network_ready || !game_data->network_manager->server)
	{
		console->error(
			"Tried to send networked entities to clients "
			" but failed. network_ready = {}, server = {}",
			network_ready,
			game_data->network_manager->server);
	}

	Packet p({PacketType::ENTITY_CREATION_ON_CONNECTION});

	auto ents = getEntitiesFromRoot();

	for (auto ent : ents)
	{
		auto& info = ent->info;
		if (info.ID <= 0)
			continue;

		if (info.name.empty() || info.type == 0)
		{
			console->error(
				"Invalid info when sending on connection of "
				"client {} for entity.\n\t{}",
				client_id,
				info);
		}
		else
		{
			console->info(
				"Sending networked entity to client {}."
				"\n\t{}",
				client_id,
				info);
		}

		p << info;
		ent->serializeOnConnection(p);
		game_data->network_manager->server->sendPacketToOneClient(client_id, 0, &p);
		p.clear();
	}
}

void Scenetree::receivedPacketFromClient(Packet p)
{
	switch (p.getHeader().type)
	{
		case CLIENT_INITIALIZED:
		{
			break;
		}

		case CONNECTED:
		{
			sendAllNetworkedEntitiesToClient(p.info.senderID);
			break;
		}

		case DISCONNECTED:
		{
			break;
		}

		case GLOBAL_RPC:
		{
			break;
		}

		case ENTITY_RPC:
		{
			receivedEntityRPCFromClient(p);
			break;
		}

		case CLASS_RPC:
		{
			break;
		}

		case ENTITY_CREATION_REQUEST:
		{
			//todo: let user plug in a predicate for if we accept the request

			//todo: create ENTITY_CREATION_REQUEST_SERVEROWNER
			//one less bit of data to send since we reuse the packet type instead
			auto type = p.read<EntityType>();
			auto name = p.read<std::string>();
			auto parentID = p.read<EntityID>();
			auto spawnInfo = p.read<Packet>();
			auto children = p.read<std::vector<EntityChildCreationInfo>>();

			createEntityNetworkedFromRequest(
				{type, name, 0, p.info.senderID, parentID},
				spawnInfo,
				children);
			break;
		}

		case ENTITY_CREATION_ON_CONNECTION:
		{
			console->error(
				"Server received ENTITY_CREATION_ON_CONNECTION from client {}",
				p.info.senderID);
			break;
		}

		case ENTITY_UPDATE:
		{
			//Don't send entity updates back to the sender
			game_data->network_manager->server->sendPacketToAllExceptOneClient(
				p.info.senderID, 0, &p);
			//game_data->network_manager->server->sendPacketToAllClients(
			//	p.info.senderID, &p);
			break;
		}

		case ENTITY_DELETION:
		{
			receivedEntityDeletionFromClient(p);
			break;
		}

		case NONE:
		case COMMAND:
		{
			break;
		}
	}
}

void Scenetree::receivedEntityRPCFromClient(Packet& p)
{
	auto info = p.read<EntityInfo>();
	auto name = p.read<std::string>();
	auto rpctype = rpc_man.getEntityRPCType(info.type, name);

	auto ent = findEntity(info.ID);
	if (!ent)
	{
		console->error(
			"Received request to call RPC {} on entity {} "
			"but it doesn't exist",
			name,
			info);
		return;
	}

	if (info != ent->info)
	{
		console->error(
			"Received request to call RPC {} on entity {} "
			"but it doesn't match the entity we have: {}",
			name,
			info,
			ent->info);
		return;
	}

	//todo: check to see if rpctype and name match our records

	switch (rpctype)
	{
		case LOCAL:
		{
			console->error(
				"Received a request for a "
				"Local Entity RPC call from {} for {} "
				"but local calls shouldn't be "
				"sent over the network",
				p.info.senderID,
				info);
			break;
		}

		case MASTER:
		{
			if (info.ownerID == p.info.senderID)
			{
				//todo: not sure if this is an error or not
				console->error(
					"Received a request for a "
					"Master Entity RPC call from {} "
					"but the sender is the owner of {}",
					p.info.senderID,
					info);
			}
			else
			{
				//only send packet to owner
				game_data->network_manager->server->sendPacketToOneClient(
					info.ownerID, 0, &p);
			}
			break;
		}

		case REMOTE:
		case REMOTE_AND_LOCAL:
		{
			if (info.ownerID == p.info.senderID)
			{
				//only send packets to non-owners, which must be all except sender
				game_data->network_manager->server->sendPacketToAllExceptOneClient(
					p.info.senderID, 0, &p);
			}
			else
			{
				console->error(
					"Received request for Remote "
					"or RemoteAndLocal RPC call from {} "
					"but the sent entity's owner ID {} "
					"doesn't match the sender's ID {}. "
					"Entity is {}",
					info.ownerID,
					p.info.senderID,
					info);
			}
			break;
		}

		case MASTER_AND_REMOTE:
		case ALL:
		{
			//send to everyone else
			game_data->network_manager->server->sendPacketToAllExceptOneClient(
				p.info.senderID, 0, &p);
			break;
		}

		default:
		{
			console->error(
				"Received request for an Entity RPC "
				"from {} for {} but the RPC type is invalid",
				p.info.senderID,
				info);
			break;
		}
	}
}

void Scenetree::receivedEntityDeletionFromClient(Packet& p)
{
	auto entID = p.read<EntityID>();
	auto ent = findEntity(entID);
	if (!ent)
	{
		console->error(
			"Received request to delete entity {} "
			"but it doesn't exist",
			entID);
		return;
	}

	if (ent->info.ownerID != p.info.senderID)
	{
		console->error(
			"Received request to delete entity {} "
			"but the sender {} isn't the owner",
			ent->info,
			p.info.senderID);
		return;
	}

	game_data->network_manager->server->sendPacketToAllClients(0, &p);
}

void Scenetree::receivedPacketFromServer(Packet p)
{
	switch (p.getHeader().type)
	{
		case CLIENT_INITIALIZED:
		{
			break;
		}

		case CONNECTED:
		{
			break;
		}

		case DISCONNECTED:
		{
			break;
		}

		case GLOBAL_RPC:
		{
			break;
		}

		case ENTITY_RPC:
		{
			auto info = p.read<EntityInfo>();
			auto ent = findEntity(info.ID);

			if (!ent)
			{
				console->error(
					"Received an RPC packet from {} "
					"for an entity that does not exist."
					"\n\t{}",
					p.info.senderID,
					info);
				return;
			}

			if (info != ent->info)
			{
				console->error(
					"Received an RPC packet from {} for an entity "
					"that does not match our version."
					"\nTheirs: \t{}"
					"\nOurs: \t{}",
					p.info.senderID,
					info,
					ent->info);
				return;
			}

			p.resetReadPosition();
			rpc_man.receive(p, ent);
			break;
		}

		case CLASS_RPC:
		{
			break;
		}

		case ENTITY_CREATION_REQUEST:
		{
			console->error("Client received ENTITY_CREATION_REQUEST from server");
			break;
		}

		case ENTITY_CREATION_TREE:
		{
			//todo: this only happens when a new entity is created on the server
			//for on-connection, it's a different enum, silly
			createEntitiesFromTreePacket(std::move(p));
			break;
		}

		case ENTITY_CREATION_ON_CONNECTION:
		{
			//auto info = p.read<EntityInfo>();
			//auto ent = createEntityNetworkedFromRequest(info, {}, {});	//todo: children/packet
			//ent->deserializeOnConnection(p);

			//todo: this is the on-connection stuff :)
			//so we'll need to grab info, deserialize on connection, and loop until reaching the end
			//onSpawn doesn't get called here though, not sure if that's okay or not
			//we would need to save the spawn info for every networked entity on the server and resend it when someone new connects if not
			//also, what order is deserialization called? no clue, does it matter? no clue, but probably
			break;
		}

		case ENTITY_UPDATE:
		{
			/* //todo: user has some way of overriding expected size, as sizeof() is only accurate for POD (and even then iffy, i.e compressed floats, all depends on how it gets serialized)
			 * //but even then iffy, because what about vector? it's variable, so it would be a minimum size requirement at best in that case, or max in the compressed case, so... useless?
			if (!p.canDeserialize<EntityInfo>())
			{
				console->error(
					"Received entity update with an invalid packet{}");
				return;
			}*/

			EntityInfo info;
			try
			{
				p >> info;
			}
			catch (...)
			{
				console->error("Received entity update with an invalid packet as it failed to deserialize EntityInfo {}", info);
				return;
			}

			auto ent = findEntity(info.ID);
			if (!ent)
			{
				console->error(
					"Received entity update for nonexistant entity."
					"\n\t{}",
					info);
				return;
			}

			if (info != ent->info)
			{
				console->error(
					"Received entity update with "
					"invalid info.\n\t{}\n\tVS\n\t{}"
					"\n\tSender ID = {}, Client ID = {}",
					info,
					ent->info,
					p.info.senderID,
					game_data->network_manager->client->getID());
				return;
			}

			ent->deserializeOnTick(p);
			break;
		}

		case ENTITY_DELETION:
		{
			auto entID = p.read<EntityID>();
			auto ent = findEntity(entID);
			if (ent)
			{
				ent->remove = true;
			}
			else
			{
				console->error(
					"Received a request for entity deletion from {} "
					"but an entity with ID {} does not exist.",
					p.info.senderID,
					entID);
			}
			break;
		}

		case NONE:
		case COMMAND:
		{
			break;
		}
	}
}

void Scenetree::receivedEntityCreationFromServer(Packet p)
{
	//todo
	//so the server will create all the networked entities from our request
	//and give them ids and then send them all to us, yay
	//and now we receive them once packet at a time (?, maybe concatenate)
	//so now we do not instantiate any children for these entities
	//they will be sent to us specifically

	if (p.info.senderID == game_data->network_manager->client->getID())
	{
		//todo: handle this better, no need to even send the packet, etc
		return;	//we're both the client and server so the server has already created this for us
	}

	while (p.canDeserialize<EntityInfo, Packet>())
	{
		auto info = p.read<EntityInfo>();
		auto spawnInfo = p.read<Packet>();

		auto [local, version, index] = splitID(info.ID);
		auto type = info.type;
		auto e = registeredTypes[type](std::move(info), game_data);

		if (index >= entitiesNetworked.size())	//need to create a new one
		{
			while (index != entitiesNetworked.size())	//need to create empty ents to match required index
			{
				entitiesNetworked.emplace_back(VersionEntityPair{0, nullptr});
			}

			//now index == size, so last push back is the correct index
			entitiesNetworked.emplace_back(VersionEntityPair{version, std::move(e)});
		}
		else
		{
			if (entitiesNetworked[index].version != version)
			{
				throw;
			}
			else if (entitiesNetworked[index].entity != nullptr)
			{
				throw;
			}

			entitiesNetworked[index] = {version, std::move(e)};
		}
	}
}

void Scenetree::onNetworkTick()
{
	total_network_ticks++;

	enki::Packet p({enki::PacketType::ENTITY_UPDATE});

	auto entities = getEntitiesFromRoot();

	//todo: some kind of flag when we iterate over entities and those entities could modify the entity container
	//so that they don't accidentally invalidate the loop
	//would be useful everywhere we call an entities non-const function inside a loop
	//in this case we grab the entities first, so subsequent changes won't matter
	//except for deletion, but that occurs next frame so probably okay

	for (auto ent : entities)
	{
		//Serialize our entities based on their specific tick rates
		if (ent->isOwner() &&
			!ent->isLocal() &&
			ent->network_tick_rate > 0 &&
			total_network_ticks % ent->network_tick_rate == 0)
		{
			p.clear();
			p << ent->info;
			ent->serializeOnTick(p);
			//todo: send lots of little packets, or one big packet?
			game_data->network_manager->client->sendPacket(0, &p);
		}
	}
}

}	// namespace enki