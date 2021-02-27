#include "Scenetree.hpp"

namespace enki
{
Scenetree::Scenetree(NetworkManager* network_manager)
	: rpc_man(network_manager)
	, m_network_manager(network_manager)
{
	if (network_manager == nullptr)
		throw;

	m_console = spdlog::get("Enki");
	if (m_console == nullptr)
		m_console = spdlog::stdout_color_mt("Enki");
}

void Scenetree::enableNetworking()
{
	if (m_network_ready)
	{
		m_console->error(
			"Tried to enable networking for scenetree "
			"but networking is already enabled");
		return;
	}

	if (!m_network_manager->isNetworked())
	{
		m_console->error(
			"Tried to enable networking for scenetree "
			"but neither the server nor the client have been started");
		return;
	}

	m_network_ready = true;

	if (m_network_manager->server)
		m_mc1 = m_network_manager->server->on_packet_received.connect(this, &Scenetree::receivedPacketFromClient);

	if (m_network_manager->client)
	{
		m_mc2 = m_network_manager->client->on_packet_received.connect(this, &Scenetree::receivedPacketFromServer);
		m_mc3 = m_network_manager->on_network_tick.connect(this, &Scenetree::onNetworkTick);
	}
}

NetworkManager* Scenetree::getNetworkManager() const
{
	return m_network_manager;
}

void Scenetree::forEachEntity(std::function<void(const Entity&)> function)
{
	for (const auto& [version, ent] : m_entities[Local])
	{
		if (ent)
			function(*ent);
	}

	for (const auto& [version, ent] : m_entities[Networked])
	{
		if (ent)
			function(*ent);
	}
}

void Scenetree::forEachEntity(std::function<void(const Entity&)> function, const std::vector<EntityID> ids)
{
	std::vector<Entity*> ents;
	for (auto id : ids)
	{
		auto* e = _findEntity(id);
		if (e)
			function(*e);
	}
}

Entity* Scenetree::createEntityLocal(const EntityType type,
	std::string name,
	const EntityID parentID,
	Packet spawnInfo,
	const std::vector<EntityChildCreationInfo>& children)
{
	if (parentID != Entity::InvalidID && _findEntity(parentID) == nullptr)
		return nullptr;
	
	if (m_registered_types[type] == nullptr)
		return nullptr;

	Entity* entity = nullptr;

	//no free indices available, create a new entity with version 0
	if (m_entities_free_indices[Local].empty())
	{
		EntityInfo info;
		info.type = type;
		info.name = std::move(name);
		info.ID = generateEntityID(true, 0, static_cast<std::uint32_t>(m_entities[Local].size()));
		info.parentID = parentID;

		if (parentID == Entity::InvalidID)
			m_entities_parentless.push_back(info.ID);

		auto e = m_registered_types[type](std::move(info));
		entity = e.get();

		m_entities[Local].emplace_back(VersionEntityPair{0, std::move(e)});
	}
	else	//reuse existing index
	{
		std::uint32_t index = m_entities_free_indices[Local].top();
		m_entities_free_indices[Local].pop();

		EntityInfo info;
		info.type = type;
		info.name = std::move(name);
		//version was incremented when the last entity here was deleted, no need to change it here
		info.ID = generateEntityID(true, m_entities[Local][index].version, index);
		info.parentID = parentID;

		if (parentID == Entity::InvalidID)
			m_entities_parentless.push_back(info.ID);

		auto e = m_registered_types[type](std::move(info));
		entity = e.get();

		m_entities[Local][index].entity = std::move(e);
	}

	for (const auto& child : m_registered_child_creation_info[type])
	{
		const auto* c = createEntityLocal(child.type, child.name, entity->info.ID, child.spawnInfo, child.children);
		entity->info.childIDs.push_back(c->info.ID);
	}

	for (const auto& child : children)
	{
		//todo: all of this copying is expensive, maybe we can move it out instead of taking a const ref?
		//not sure how well that would fit in to the usecases of passing spawninfo on construction, we'll see
		auto* c = createEntityLocal(child.type, child.name, entity->info.ID, child.spawnInfo, child.children);
		entity->info.childIDs.push_back(c->info.ID);
	}

	entity->onSpawn(std::move(spawnInfo));

	return entity;
}

void Scenetree::createEntityNetworkedRequest(const EntityType type, std::string name, const EntityID parentID, Packet spawnInfo, const std::vector<EntityChildCreationInfo>& children)
{
	auto net_man = m_network_manager;
	if (!m_network_ready)
	{
		m_console->error(
			"Tried to request a networked entity "
			"when scenetree isn't network ready");
		return;
	}

	if (!net_man->isClient())
	{
		m_console->error(
			"Tried to request a networked entity "
			"when there is no client");
		return;
	}

	if (name.empty() || type == 0)
	{
		m_console->error(
			"Invalid info when creating networked entity."
			"\nname: {}, type: {}",
			name,
			type);
		return;
	}

	if (!m_registered_types.count(type))
	{
		m_console->error(
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

	m_console->info(
		"Sending networked entity request as the client "
		"to the server.\n\t{}",
		EntityInfo{type, name, 0, 0, parentID});
	net_man->client->sendPacket(0, &p);
}

Scenetree::ErrorCodeRemove Scenetree::removeEntity(const EntityID ID)
{
	if (localFromID(ID))
		return removeEntityLocal(ID);

	return ErrorCodeRemove::Unknown;
}

Scenetree::ErrorCodeRemove Scenetree::removeEntityLocal(const EntityID ID)
{
	if (ID == 0)
		return ErrorCodeRemove::IDWasZero;

	const auto [local, version, index] = splitID(ID);

	if (!local)
		return ErrorCodeRemove::IDWasNotLocal;
	
	if (index >= m_entities[Local].size())
		return ErrorCodeRemove::IndexOutOfBounds;

	auto& e = m_entities[Local][index];
	if (ID != e.entity->info.ID)
		return ErrorCodeRemove::IDDoesNotMatchFoundID;
	//this should never happen, the version we store and the version in the ID has diverged somehow
	if (version != e.version)
		std::abort();

	e.entity->onDespawn();

	for (auto c : e.entity->info.childIDs)
		removeEntityLocal(c);

	e.version++;
	e.entity.reset(nullptr);
	m_entities_free_indices[Local].push(index);
	return ErrorCodeRemove::Success;
}

Entity* Scenetree::findEntity(const EntityID ID) const
{
	return _findEntity<0>(ID);
}

Entity* Scenetree::_findEntity(EntityID ID) const
{
	return _findEntity<1>(ID);
}

Entity* Scenetree::getEntityUnsafe(const EntityID ID)
{
	return m_entities[ID < 0][indexFromID(ID)].entity.get();
}

bool Scenetree::registerChildren(const EntityType type, std::vector<EntityChildCreationInfo> children)
{
	if (!children.empty())
	{
		std::set<EntityType> parentTypes{type};

		if (!checkChildrenValid(parentTypes, children))
			return false;

		m_registered_child_creation_info[type] = children;	 //do we need to do this beforehand?
	}

	return true;
}

std::vector<Entity*> Scenetree::getEntitiesFromRoot(EntityID ID)
{
	std::vector<Entity*> ents;

	if (ID == 0)
	{
		fillEntitiesFromChildren(nullptr, m_entities_parentless, ents);
	}
	else
	{
		auto* e = _findEntity(ID);
		if (e)
			fillEntitiesFromChildren(e, e->info.childIDs, ents);
	}

	return ents;
}

void Scenetree::input(Event& event)
{
	// do not use range for loop
	for (std::size_t i = 0; i < m_entities_parentless.size(); ++i)
		input(event, m_entities_parentless[i]);
}

void Scenetree::update(float dt)
{
	auto entities = getEntitiesFromRoot();
	std::vector<Entity*> entitiesToRemove;

	//todo: does not handle deleting children of deleted entities
	//mark as deleted automatically when doing the deletion?

	//from parents to children
	for (auto* e : entities)
	{
		if (e && e->remove)
		{
			auto [local, version, index] = splitID(e->info.ID);
			this->m_entities[local][index].entity->onDespawn();
			entitiesToRemove.push_back(e);
		}
	}

	//from children to parents
	for (auto it = entitiesToRemove.rbegin(); it != entitiesToRemove.rend(); ++it)
	{
		auto* e = *it;
		auto [local, version, index] = splitID(e->info.ID);

		//todo: faster/correct place?
		auto* parent = _findEntity(e->info.parentID);
		if (parent)
		{
			std::erase_if(parent->info.childIDs, [removedID = e->info.ID](EntityID id) {
				return id == removedID;
			});
		}
		
		std::erase_if(m_entities_parentless, [removedID = e->info.ID](EntityID id) {
			return id == removedID;
		});

		this->m_entities[local][index].version++;
		this->m_entities[local][index].entity = nullptr;
		m_entities_free_indices[local].push(index);
	}

	//do not use range for loop
	for (unsigned int i = 0; i < m_entities_parentless.size(); ++i)
		update(dt, m_entities_parentless[i]);
}

void Scenetree::draw(Renderer* renderer)
{
	//do not use range for loop
	for (unsigned int i = 0; i < m_entities_parentless.size(); ++i)
		draw(renderer, m_entities_parentless[i]);
}

void Scenetree::deleteEntity(EntityID ID)
{
	auto* e = _findEntity(ID);

	if (!e)
	{
		m_console->error(
			"Tried to delete entity {} "
			"but it doesn't exist",
			ID);
		return;
	}

	if (localFromID(e->info.ID))
	{
		e->remove = true;
	}
	else if (m_network_ready)
	{
		Packet p({ENTITY_DELETION});
		p << ID;

		if (m_network_manager->isServer())
			m_network_manager->server->sendPacketToAllClients(0, &p);
		else if (m_network_manager->isClient())
			m_network_manager->client->sendPacket(0, &p);
	}
}

std::vector<Entity*> Scenetree::findEntitiesByType(HashedID type) const
{
	std::vector<Entity*> ents;

	for (const auto& group : m_entities)
		for (const auto& [version, entity] : group)
			if (entity && entity->info.type == type)
				ents.push_back(entity.get());

	return ents;
}

std::vector<Entity*> Scenetree::findEntitiesByName(const std::string& name) const
{
	std::vector<Entity*> ents;

	for (const auto& group : m_entities)
		for (const auto& [version, entity] : group)
			if (entity && entity->info.name == name)
				ents.push_back(entity.get());

	return ents;
}

std::vector<Entity*> Scenetree::findEntitiesByOwner(ClientID owner) const
{
	std::vector<Entity*> ents;

	for (const auto& group : m_entities)
		for (const auto& [version, entity] : group)
			if (entity && entity->info.ownerID == owner)
				ents.push_back(entity.get());

	return ents;
}

std::vector<Entity*> Scenetree::findEntitiesByParent(EntityID parent) const
{
	std::vector<Entity*> ents;

	for (const auto& group : m_entities)
		for (const auto& [version, entity] : group)
			if (entity && entity->info.parentID == parent)
				ents.push_back(entity.get());
	
	return ents;
}

std::vector<Entity*> Scenetree::findEntitiesByPredicate(const std::function<bool(const Entity&)>& predicate) const
{
	std::vector<Entity*> ents;

	for (const auto& group : m_entities)
		for (const auto& [version, entity] : group)
			if (entity && predicate(*entity))
				ents.push_back(entity.get());

	return ents;
}

void Scenetree::createEntityNetworkedFromRequest(EntityInfo info,
	const Packet& spawnInfo,
	const std::vector<EntityChildCreationInfo>& children)
{
	Packet p({PacketType::ENTITY_CREATION_TREE});
	auto* e = createEntityNetworkedFromRequestImpl(std::move(info), spawnInfo, children, p);

	m_console->info(
		"Creating networked entity as the server "
		"to all clients. Sending tree\n\t{}",
		e->info);
	//Don't forward to ourselves in case we're hosting
	//todo: maybe think about having a different ID even when hosting, i.e. 0 is invalid, 1 is this server, 2 is our client, 3+ is everyone else
	m_network_manager->server->sendPacketToAllExceptOneClient(1, 0, &p);
}

////////////////////////
/////////////////PRIVATE
////////////////////////

Entity* Scenetree::createEntityNetworkedFromRequestImpl(EntityInfo info,
	const Packet& spawnInfo,
	const std::vector<EntityChildCreationInfo>& children,
	Packet& p)
{
	if (info.name.empty() || info.type == 0)
	{
		m_console->error(
			"Invalid info when creating networked entity."
			"\n\t{}",
			info);
		throw;
	}

	if (!m_registered_types.count(info.type))
	{
		m_console->error(
			"Tried to create networked entity without "
			"registering it first.\n\t{}",
			info);
		throw;
	}

	if (info.ID < 0)
	{
		m_console->error(
			"Tried to create networked entity that has "
			"a local ID.\n\t{}",
			info);
		throw;
	}

	if (info.ID > 0)
	{
		m_console->error(
			"Tried to create networked entity that has "
			"an existing ID.\n\t{}",
			info);
		throw;
	}

	if (info.parentID != Entity::InvalidID && !_findEntity(info.parentID))
	{
		m_console->error(
			"Tried to create networked entity that has "
			"the non-existent parent {} specified.\n\t{}",
			prettyID(info.parentID),
			info);
		throw;
	}

	std::uint32_t index;
	std::uint32_t version;

	if (m_entities_free_indices[Networked].empty())
	{
		index = static_cast<std::uint32_t>(m_entities[Networked].size());
		version = 0;
		m_entities[Networked].emplace_back(VersionEntityPair{0, nullptr});
	}
	else
	{
		index = m_entities_free_indices[Networked].top();
		m_entities_free_indices[Networked].pop();
		version = m_entities[Networked][index].version;
	}

	info.ID = generateEntityID(false, version, index);

	if (info.parentID == Entity::InvalidID)
	{
		m_entities_parentless.push_back(info.ID);
	}
	else
	{
		auto* parent = findEntity(info.parentID);
		assert(parent);
		parent->info.childIDs.push_back(info.ID);
	}

	Entity* entity;

	{
		auto e = m_registered_types[info.type](info);
		entity = e.get();
		m_entities[Networked][index].version = version;
		m_entities[Networked][index].entity = std::move(e);
	}

	p << entity->info;
	p << spawnInfo;

	//todo: both of these for loops have the same body
	for (auto& child : m_registered_child_creation_info[info.type])
	{
		auto* c = createEntityNetworkedFromRequestImpl(
			{child.type, child.name, Entity::InvalidID, info.ownerID, info.ID},
			child.spawnInfo,
			child.children,
			p);
		p << c->info;
		p << child.spawnInfo;
	}

	for (const auto& child : children)
	{
		auto* c = createEntityNetworkedFromRequestImpl(
			{child.type, child.name, Entity::InvalidID, info.ownerID, info.ID},
			child.spawnInfo,
			child.children,
			p);
		p << c->info;
		p << child.spawnInfo;
	}

	entity->onSpawn(spawnInfo);
	return entity;
}

void Scenetree::createEntitiesFromTreePacket(Packet p)
{
	m_console->info("got tree packet {}", p.getBytesWritten());

	if (!m_network_manager->isClient())
	{
		m_console->error("packet tree we are the server");
		throw;
	}

	try
	{
		auto info = p.read<EntityInfo>();
		const auto spawnInfo = p.read<Packet>();

		auto [local, version, index] = splitID(info.ID);
		const auto type = info.type;

		if (info.parentID == Entity::InvalidID)
		{
			m_entities_parentless.push_back(info.ID);
		}
		else
		{
			auto* parent = findEntity(info.parentID);
			assert(parent);
			parent->info.childIDs.push_back(info.ID);
		}

		auto e = m_registered_types[type](std::move(info));
		auto* entity = e.get();

		if (index >= m_entities[Networked].size())	//need to create a new one
		{
			m_console->info(
				"Creating additional entities from a tree packet. {} {}\n{}",
				index,
				m_entities[Networked].size(),
				entity->info);

			while (index != m_entities[Networked].size())	   //need to create empty ents to match required index
				m_entities[Networked].emplace_back(VersionEntityPair{0, nullptr});

			//now index == size, so last push back is the correct index
			m_entities[Networked].emplace_back(VersionEntityPair{version, std::move(e)});
		}
		else
		{
			m_console->info(
				"Creating exact entity from a tree packet. {} {}\n{}",
				index,
				m_entities[Networked].size(),
				entity->info);

			if (m_entities[Networked][index].version != version												 //version mismatch
				&& (m_entities[Networked][index].entity != nullptr && m_entities[Networked][index].version != 0))	 //but our version is 0 with a null entity, so it's just a placeholder, making it okay
			{
				m_console->error("packet tree version mismatch");
				//throw;
			}
			else if (m_entities[Networked][index].entity != nullptr)
			{
				m_console->error("packet tree entity already exists at index with same version");
				//throw;
			}

			m_entities[Networked][index] = {version, std::move(e)};
		}

		//todo: this is calling it from parent-to-child, should be child-to-parent
		entity->onSpawn(spawnInfo);
	}
	catch (...)
	{
		m_console->info(
			"Failed to create entities from a tree packet");
		//todo: this is a shit way of handling this problem
	}
}

void Scenetree::input(Event& event, EntityID ID)
{
	auto* e = _findEntity(ID);
	if (!e)
		return;
	
	e->input(event);
	for (const auto childID : e->info.childIDs)
		input(event, childID);
}

void Scenetree::update(float dt, EntityID ID)
{
	auto* e = _findEntity(ID);
	if (!e)
		return;

	e->update(dt);
	for (const auto childID : e->info.childIDs)
		update(dt, childID);
}

void Scenetree::draw(Renderer* renderer, EntityID ID)
{
	auto* e = _findEntity(ID);
	if (!e)
		return;
	
	e->draw(renderer);
	for (const auto childID : e->info.childIDs)
		draw(renderer, childID);
}

void Scenetree::fillEntitiesFromChildren(Entity* parent, std::vector<EntityID> children, std::vector<Entity*>& ents)
{
	for (auto id : children)
	{
		auto* e = _findEntity(id);
		if (e)
		{
			ents.push_back(e);
			fillEntitiesFromChildren(e, e->info.childIDs, ents);
		}
		else
		{
			if (parent)
			{
				spdlog::get("Enki")->warn("Entity {} no longer exists, but is a child of another entity.\n{}",
					prettyID(id),
					parent->info);
			}
			else
			{
				spdlog::get("Enki")->warn("Entity {} no longer exists\n",
					prettyID(id));
			}
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
		if (!m_registered_child_creation_info[child.type].empty())
		{
			parentTypes.insert(child.type);
			const bool valid = checkChildrenValid(parentTypes, m_registered_child_creation_info[child.type]);
			parentTypes.erase(child.type);
			if (!valid)
				return false;
		}

		//check unregistered grandchildren against our parental hierarchy and ourselves
		if (!child.children.empty())
		{
			parentTypes.insert(child.type);
			const bool valid = checkChildrenValid(parentTypes, child.children);
			parentTypes.erase(child.type);
			if (!valid)
				return false;
		}
	}

	return true;
}


void Scenetree::sendAllNetworkedEntitiesToClient(ClientID client_id)
{
	if (!m_network_ready || !m_network_manager->isServer())
	{
		m_console->error(
			"Tried to send networked entities to clients "
			" but failed. network_ready = {}, server = {}",
			m_network_ready,
			m_network_manager->isServer());
	}

	Packet p({PacketType::ENTITY_CREATION_ON_CONNECTION});

	//flat tree structure
	auto ents = getEntitiesFromRoot();

	for (auto* ent : ents)
	{
		auto& info = ent->info;
		
		//todo: a local entity could have networked children, which is a bit odd I feel like, yet kinda makes sense in terms of local groupings/states
		//these children will have an incorrect parent ID, as it will be local to the specific instance. dunno if there's a good solution for this.
		if (info.ID <= 0)
			continue;

		if (info.name.empty() || info.type == 0)
		{
			m_console->error(
				"Invalid info when sending on connection of "
				"client {} for entity.\n\t{}",
				client_id,
				info);
		}
		else
		{
			m_console->info(
				"Sending networked entity to client {}."
				"\n\t{}",
				client_id,
				info);
		}

		auto test = p.writeAndRetrieve<EntityInfo>(info);

		if (test != info)
		{
			assert(false);
		}

		ent->serializeOnConnection(p);
	}
	m_network_manager->server->sendPacketToOneClient(client_id, 0, &p);
}

void Scenetree::receivedPacketFromClient(Packet p)
{
	switch (p.getHeader().type)
	{
		case CLIENT_INITIALIZED:
		case DISCONNECTED:
		case GLOBAL_RPC:
		case CLASS_RPC:
		case NONE:
		case COMMAND:
		{
			break;
		}
		
		case CONNECTED:
		{
			sendAllNetworkedEntitiesToClient(p.info.senderID);
			break;
		}

		case ENTITY_RPC:
		{
			receivedEntityRPCFromClient(p);
			break;
		}

		case ENTITY_CREATION_REQUEST:
		{
			//todo: let user plug in a predicate for if we should accept the request
			// e.g. disable certain clients from creating certain entities

			//todo: create ENTITY_CREATION_REQUEST_SERVEROWNER
			//one less bit of data to send since we reuse the packet type instead

			//todo: we should create structs to represent the data we'd expect in all of our packet types
			const auto type = p.read<EntityType>();
			auto name = p.read<std::string>();
			const auto parentID = p.read<EntityID>();
			const auto spawnInfo = p.read<Packet>();
			const auto children = p.read<std::vector<EntityChildCreationInfo>>();

			createEntityNetworkedFromRequest(
				{type, std::move(name), 0, p.info.senderID, parentID},
				spawnInfo,
				children);
			break;
		}

		case ENTITY_CREATION_ON_CONNECTION:
		{
			m_console->error(
				"Server received ENTITY_CREATION_ON_CONNECTION from client {}",
				p.info.senderID);
			break;
		}

		case ENTITY_UPDATE:
		{
			//Don't send entity updates back to the sender
			m_network_manager->server->sendPacketToAllExceptOneClient(
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
	}
}

void Scenetree::receivedEntityRPCFromClient(Packet& p)
{
	const auto info = p.read<EntityInfo>();
	const auto name = p.read<std::string>();
	const auto rpctype = rpc_man.getEntityRPCType(info.type, name);

	const auto* ent = _findEntity(info.ID);
	if (!ent)
	{
		m_console->error(
			"Received request to call RPC {} on entity {} "
			"but it doesn't exist",
			name,
			info);
		return;
	}

	if (info != ent->info)
	{
		m_console->error(
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
			m_console->error(
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
				m_console->error(
					"Received a request for a "
					"Master Entity RPC call from {} "
					"but the sender is the owner of {}",
					p.info.senderID,
					info);
			}
			else
			{
				//only send packet to owner
				m_network_manager->server->sendPacketToOneClient(
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
				m_network_manager->server->sendPacketToAllExceptOneClient(
					p.info.senderID, 0, &p);
			}
			else
			{
				m_console->error(
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
			m_network_manager->server->sendPacketToAllExceptOneClient(
				p.info.senderID, 0, &p);
			break;
		}

		default:
		{
			m_console->error(
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
	const auto entID = p.read<EntityID>();
	const auto* ent = _findEntity(entID);
	if (!ent)
	{
		m_console->error(
			"Received request to delete entity {} "
			"but it doesn't exist",
			entID);
		return;
	}

	if (ent->info.ownerID != p.info.senderID)
	{
		m_console->error(
			"Received request to delete entity {} "
			"but the sender {} isn't the owner",
			ent->info,
			p.info.senderID);
		return;
	}

	m_network_manager->server->sendPacketToAllClients(0, &p);
}

void Scenetree::receivedPacketFromServer(Packet p)
{
	switch (p.getHeader().type)
	{
		case CLIENT_INITIALIZED:
		case CONNECTED:
		case DISCONNECTED:
		case GLOBAL_RPC:
		case CLASS_RPC:
		case NONE:
		case COMMAND:
		{
			break;
		}

		case ENTITY_RPC:
		{
			auto info = p.read<EntityInfo>();
			auto* ent = _findEntity(info.ID);

			if (!ent)
			{
				m_console->error(
					"Received an RPC packet from {} "
					"for an entity that does not exist."
					"\n\t{}",
					p.info.senderID,
					info);
				return;
			}

			if (info != ent->info)
			{
				m_console->error(
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

		case ENTITY_CREATION_REQUEST:
		{
			m_console->error("Client received ENTITY_CREATION_REQUEST from server");
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

			//todo: this is prob. not correct/complete, just hacky
			while (p.getBytesRead() < p.getBytesWritten())
			{
				//todo: check version/index can be used correctly
				auto info = p.read<EntityInfo>();
				auto [local, version, index] = splitID(info.ID);
				
				if (info.parentID == Entity::InvalidID)
				{
					m_entities_parentless.push_back(info.ID);
				}

				auto e = m_registered_types[info.type](info);
				auto* entity = e.get();

				if (index >= m_entities[Networked].size())	//need to create a new one
				{
					m_console->info(
						"Creating additional entities from a connection packet. {} {}\n{}",
						index,
						m_entities[Networked].size(),
						info);

					while (index != m_entities[Networked].size())	   //need to create empty ents to match required index
						m_entities[Networked].emplace_back(VersionEntityPair{0, nullptr});

					//now index == size, so last push back is the correct index
					m_entities[Networked].emplace_back(VersionEntityPair{version, std::move(e)});
				}
				else
				{
					m_console->info(
						"Creating exact entity from a connection packet. {} {}\n{}",
						index,
						m_entities[Networked].size(),
						info);

					if (m_entities[Networked][index].version != version												 //version mismatch
						&& (m_entities[Networked][index].entity != nullptr && m_entities[Networked][index].version != 0))	 //but our version is 0 with a null entity, so it's just a placeholder, making it okay
					{
						m_console->error("packet connection version mismatch");
						//throw;
					}
					else if (m_entities[Networked][index].entity != nullptr)
					{
						m_console->error("packet connection entity already exists at index with same version");
						//throw;
					}

					m_entities[Networked][index] = {version, std::move(e)};
				}

				//todo: we pass the whole damn thing, they should only have access to their specific data, this is really really bad
				entity->deserializeOnConnection(p);
			}

			break;
		}

		case ENTITY_UPDATE:
		{
			/* //todo: user has some way of overriding expected size, as sizeof() is only accurate for POD (and even then iffy, i.e compressed floats, all depends on how it gets serialized)
			 * //but even then iffy, because what about vector? it's variable, so it would be a minimum size requirement at best in that case, or max in the compressed case, so... useless?
			if (!p.canDeserialize<EntityInfo>())
			{
				m_console->error(
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
				m_console->error("Received entity update with an invalid packet as it failed to deserialize EntityInfo {}", info);
				return;
			}

			auto* ent = _findEntity(info.ID);
			if (!ent)
			{
				m_console->error(
					"Received entity update for nonexistant entity."
					"\n\t{}",
					info);
				return;
			}

			if (info != ent->info)
			{
				m_console->error(
					"Received entity update with "
					"invalid info.\n\t{}\n\tVS\n\t{}"
					"\n\tSender ID = {}, Client ID = {}",
					info,
					ent->info,
					p.info.senderID,
					m_network_manager->client->getID());
				return;
			}

			ent->deserializeOnTick(p);
			break;
		}

		case ENTITY_DELETION:
		{
			auto entID = p.read<EntityID>();
			auto* ent = _findEntity(entID);
			if (ent)
			{
				ent->remove = true;
			}
			else
			{
				m_console->error(
					"Received a request for entity deletion from {} "
					"but an entity with ID {} does not exist.",
					p.info.senderID,
					entID);
			}
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

	//todo: handle this better, no need to even send the packet, etc
	//we're both the client and server so the server has already created this for us
	if (p.info.senderID == m_network_manager->client->getID())
		return;

	//todo: we should send how many entities and use a for loop and then check at the end that there's nothing left in the packet
	//while (p.canDeserialize<EntityInfo, Packet>())
	while (p.getBytesRead() < p.getBytesWritten())
	{
		auto info = p.read<EntityInfo>();
		auto spawnInfo = p.read<Packet>();

		auto [local, version, index] = splitID(info.ID);
		auto type = info.type;
		auto e = m_registered_types[type](std::move(info));

		if (index >= m_entities[Networked].size())	//need to create a new one
		{
			while (index != m_entities[Networked].size())	   //need to create empty ents to match required index
				m_entities[Networked].emplace_back(VersionEntityPair{0, nullptr});

			//now index == size, so last push back is the correct index
			m_entities[Networked].emplace_back(VersionEntityPair{version, std::move(e)});
		}
		else
		{
			if (m_entities[Networked][index].version != version
				|| m_entities[Networked][index].entity != nullptr)
				throw;

			m_entities[Networked][index] = {version, std::move(e)};
		}
	}
}

void Scenetree::onNetworkTick()
{
	m_total_network_ticks++;

	Packet p({PacketType::ENTITY_UPDATE});

	auto entities = getEntitiesFromRoot();

	//todo: some kind of flag when we iterate over entities and those entities could modify the entity container
	//so that they don't accidentally invalidate the loop
	//would be useful everywhere we call an entities non-const function inside a loop
	//in this case we grab the entities first, so subsequent changes won't matter
	//except for deletion, but that occurs next frame so probably okay
	//although let's say a relationship changes, e.g. a child becomes a sibling. depending on when this happens could cause odd behaviour
	//e.g. A->B becomes A|B, but this happened while processing A so it's okay. if there's B->C, sibling A, and we make C a child of A when we run update on B, then an update on C will not run that frame
	//I dunno, maybe that makes sense

	for (auto* ent : entities)
	{
		//Serialize our entities based on their specific tick rates
		if (ent->isOwner(m_network_manager) &&
			ent->isNetworked() &&
			ent->network_tick_rate > 0 &&
			m_total_network_ticks % ent->network_tick_rate == 0)
		{
			p.clear();
			p << ent->info;
			//todo: this is why we need a generic buffer, because sometimes we need to serialise a packet within a packet and that means redundant packet header overhead
			ent->serializeOnTick(p);
			//todo: send lots of little packets, or one big packet?
			m_network_manager->client->sendPacket(0, &p);
		}
	}
}

}	 // namespace enki