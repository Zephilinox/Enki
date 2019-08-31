#include "Scenegraph.hpp"

//STD
#include <experimental/map>

//LIBS
#include <spdlog/sinks/stdout_color_sinks.h>

//SELF
#include "Networking/ClientHost.hpp"
#include "Networking/ClientStandard.hpp"
#include "Networking/ServerHost.hpp"

namespace enki
{
Scenegraph::Scenegraph(GameData* game_data)
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

void Scenegraph::enableNetworking()
{
	if (network_ready)
	{
		console->error(
			"Tried to enable networking for scenegraph "
			"but networking is already enabled");
		return;
	}

	if (!game_data->network_manager->server &&
		!game_data->network_manager->client)
	{
		console->error(
			"Tried to enable networking for scenegraph "
			"but neither the server nor the client have been started");
		return;
	}

	network_ready = true;

	if (game_data->network_manager->server)
	{
		mc1 = game_data->network_manager->server->on_packet_received.connect(this, &Scenegraph::receivedPacketFromClient);
	}

	if (game_data->network_manager->client)
	{
		mc2 = game_data->network_manager->client->on_packet_received.connect(this, &Scenegraph::receivedPacketFromServer);

		mc3 = game_data->network_manager->on_network_tick.connect([this]() {
			total_network_ticks++;

			enki::Packet p({enki::PacketType::ENTITY_UPDATE});

			for (auto& [ID, ent] : entities)
			{
				//Serialize our entities based on their specific tick rates
				if (ent->isOwner() &&
					ent->network_tick_rate > 0 &&
					total_network_ticks % ent->network_tick_rate == 0)
				{
					p.clear();
					p << ent->info;
					ent->serializeOnTick(p);
					game_data->network_manager->client->sendPacket(0, &p);
				}
			}
		});
	}
}

void Scenegraph::input(sf::Event& e)
{
	for (auto& [ID, ent] : entities)
	{
		if (ent->info.parentID == 0)
		{
			ent->input(e);
			inputHierarchy(e, ent->info.ID);
		}
	}
}

void Scenegraph::update(float dt)
{
	for (auto& [ID, ent] : entities)
	{
		if (ent->info.parentID == 0)
		{
			ent->update(dt);
			updateHierarchy(dt, ent->info.ID);
		}
	}

	std::experimental::erase_if(entities, [](const auto& ent) {
		return ent.second->remove;
	});
}

void Scenegraph::draw(Renderer* renderer)
{
	for (const auto& [ID, ent] : entities)
	{
		if (ent->info.parentID == 0)
		{
			ent->draw(renderer);
			drawHierarchy(renderer, ent->info.ID);
		}
	}
}

void Scenegraph::registerEntity(HashedID type, BuilderFunction builder)
{
	builders[type] = std::move(builder);
}

Entity* Scenegraph::createEntity(EntityInfo info)
{
	console->info("Creating {}", info);
	Packet p;
	return createEntity(std::move(info), p);
}

void Scenegraph::createNetworkedEntity(EntityInfo info)
{
	Packet p;
	createNetworkedEntity(std::move(info), p);
}

Entity* Scenegraph::createEntity(EntityInfo info, Packet& spawnInfo)
{
	if (info.name.empty() || info.type == 0)
	{
		console->error("Invalid info when creating entity.\n\t{}", info);
		return nullptr;
	}

	if (!builders.count(info.type))
	{
		console->error(
			"Tried to create entity without "
			"registering it first.\n\t{}",
			info);
		return nullptr;
	}

	if (info.ID == 0)
	{
		info.ID = localID--;
	}

	//todo: handle child entities for local entities
	//info gets assigned to the entity here through being passed to the Entity base class constructor
	entities[info.ID] = builders.at(info.type)(info);
	entities[info.ID]->onSpawn(spawnInfo);

	return entities[info.ID].get();
}

void Scenegraph::createNetworkedEntity(EntityInfo info, Packet& spawnInfo)
{
	auto net_man = game_data->network_manager;

	if (!network_ready)
	{
		console->error(
			"Tried to create networked entity "
			"when scenegraph isn't network ready");
		return;
	}

	if (info.name.empty() || info.type == 0)
	{
		console->error(
			"Invalid info when creating networked entity."
			"\n\t{}",
			info);
		return;
	}

	if (!builders.count(info.type))
	{
		console->error(
			"Tried to create entity without "
			"registering it first.\n\t{}",
			info);
		return;
	}

	if (net_man->server && info.ID == 0)
	{
		info.ID = networkID++;
	}

	for (auto& child_info : entities_child_types[info.type])
	{
		createNetworkedEntity({child_info.type, child_info.name, 0, info.ownerID, info.ID}, child_info.spawnInfo);
	}

	if (info.ownerID == 0)
	{
		if (net_man->client)
		{
			if (net_man->client->getID() == 0)
			{
				console->error(
					"Tried to create networked entity "
					"but our ID is 0");
				return;
			}

			info.ownerID = net_man->client->getID();
		}
		else
		{
			info.ownerID = 1;
		}
	}

	Packet p({PacketType::ENTITY_CREATION});
	p << info;
	p << spawnInfo;

	if (net_man->server)
	{
		console->info(
			"Creating networked entity as the server "
			"to all clients.\n\t{}",
			info);
		net_man->server->sendPacketToAllClients(0, &p);
	}
	else if (net_man->client)
	{
		console->info(
			"Creating networked entity as the client "
			"to the server.\n\t{}",
			info);
		net_man->client->sendPacket(0, &p);
	}
	else
	{
		console->error(
			"Tried to create networked entity when "
			"network isn't running");
	}
}

void Scenegraph::forEachEntity(std::function<void(const Entity&)> function)
{
	for (const auto& [ID, ent] : entities)
	{
		function(*ent);
	}
}

void Scenegraph::inputHierarchy(sf::Event& e, EntityID parentID)
{
	for (auto& [ID, ent] : entities)
	{
		if (ent->info.parentID == parentID)
		{
			ent->input(e);
			inputHierarchy(e, ent->info.ID);
		}
	}
}

void Scenegraph::updateHierarchy(float dt, EntityID parentID)
{
	for (auto& [ID, ent] : entities)
	{
		if (ent->info.parentID == parentID)
		{
			ent->update(dt);
			updateHierarchy(dt, ent->info.ID);
		}
	}
}

void Scenegraph::drawHierarchy(enki::Renderer* renderer, EntityID parentID)
{
	for (auto& [ID, ent] : entities)
	{
		if (ent->info.parentID == parentID)
		{
			ent->draw(renderer);
			drawHierarchy(renderer, ent->info.ID);
		}
	}
}

void Scenegraph::sendAllNetworkedEntitiesToClient(ClientID client_id)
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

	//todo: replace this with hierarchy version
	for (auto i = entities.rbegin(); i != entities.rend(); ++i)
	{
		auto& [ID, ent] = *i;
		EntityInfo info = ent->info;

		if (info.ID <= 0)
		{
			continue;
		}

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

		p.clear();
		p << info;
		ent->serializeOnConnection(p);
		game_data->network_manager->server->sendPacketToOneClient(client_id, 0, &p);
	}
}

void Scenegraph::receivedPacketFromClient(Packet p)
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

		case ENTITY_CREATION:
		{
			auto info = p.read<EntityInfo>();
			auto spawnInfo = p.read<Packet>();
			info.ownerID = p.info.senderID;
			createNetworkedEntity(info, spawnInfo);
			break;
		}

		case ENTITY_CREATION_ON_CONNECTION:
		{
			console->error("Server received ENTITY_CREATION_ON_CONNECTION from client {}", p.info.senderID);
			break;
		}

		case ENTITY_UPDATE:
		{
			//Don't send entity updates back to the sender
			game_data->network_manager->server->sendPacketToAllExceptOneClient(p.info.senderID, 0, &p);
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

void Scenegraph::receivedEntityRPCFromClient(Packet& p)
{
	auto info = p.read<EntityInfo>();
	auto name = p.read<std::string>();
	auto rpctype = rpc_man.getEntityRPCType(info.type, name);

	if (!entityExists(info.ID))
	{
		console->error(
			"Received request to call RPC {} on entity {} "
			"but it doesn't exist",
			name,
			info);
		return;
	}

	auto ent = getEntity(info.ID);
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
				game_data->network_manager->server->sendPacketToOneClient(info.ownerID, 0, &p);
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

void Scenegraph::receivedEntityDeletionFromClient(Packet& p)
{
	auto entID = p.read<EntityID>();
	if (!entityExists(entID))
	{
		console->error(
			"Received request to delete entity {} "
			"but it doesn't exist",
			entID);
		return;
	}

	auto ent = getEntity(entID);
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

void Scenegraph::receivedPacketFromServer(Packet p)
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
			if (!entityExists(info.ID))
			{
				console->error(
					"Received an RPC packet from {} "
					"for an entity that does not exist."
					"\n\t{}",
					p.info.senderID,
					info);
				return;
			}

			auto ent = getEntity(info.ID);
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

		case ENTITY_CREATION:
		{
			auto info = p.read<EntityInfo>();
			auto spawnInfo = p.read<Packet>();
			createEntity(info, spawnInfo);
			break;
		}

		case ENTITY_CREATION_ON_CONNECTION:
		{
			auto info = p.read<EntityInfo>();
			auto ent = createEntity(info);
			ent->deserializeOnConnection(p);
			break;
		}

		case ENTITY_UPDATE:
		{
			auto info = p.read<EntityInfo>();
			if (!entityExists(info.ID))
			{
				console->error(
					"Received entity update for nonexistant entity."
					"\n\t{}",
					info);
				return;
			}

			auto ent = getEntity(info.ID);
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
			if (entityExists(entID))
			{
				entities[entID]->remove = true;
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

Entity* Scenegraph::getEntity(EntityID entityID) const
{
	return entities.at(entityID).get();
}

bool Scenegraph::entityExists(EntityID entityID) const
{
	return entities.count(entityID);
}

void Scenegraph::deleteEntity(EntityID entityID)
{
	if (!entityExists(entityID))
	{
		console->error(
			"Tried to delete entity {} "
			"but it doesn't exist",
			entityID);
		return;
	}

	if (entities[entityID]->info.ID < 0)
	{
		entities[entityID]->remove = true;
	}
	else if (network_ready)
	{
		Packet p({ENTITY_DELETION});
		p << entityID;

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

std::vector<Entity*> Scenegraph::findEntitiesByType(HashedID type) const
{
	std::vector<Entity*> ents;

	for (const auto& [key, value] : entities)
	{
		if (value->info.type == type)
		{
			ents.push_back(value.get());
		}
	}

	return ents;
}

std::vector<Entity*> Scenegraph::findEntitiesByName(const std::string& name) const
{
	std::vector<Entity*> ents;

	for (const auto& [key, value] : entities)
	{
		if (value->info.name == name)
		{
			ents.push_back(value.get());
		}
	}

	return ents;
}

std::vector<Entity*> Scenegraph::findEntitiesByOwner(ClientID owner) const
{
	std::vector<Entity*> ents;

	for (const auto& [key, value] : entities)
	{
		if (value->info.ownerID == owner)
		{
			ents.push_back(value.get());
		}
	}

	return ents;
}

std::vector<Entity*> Scenegraph::findEntitiesByParent(EntityID parent) const
{
	std::vector<Entity*> ents;

	for (const auto& [key, value] : entities)
	{
		if (value->info.parentID == parent)
		{
			ents.push_back(value.get());
		}
	}

	return ents;
}

std::vector<Entity*> Scenegraph::findEntitiesByPredicate(const std::function<bool(const Entity&)>& predicate) const
{
	std::vector<Entity*> ents;

	for (const auto& [key, value] : entities)
	{
		if (predicate(*value))
		{
			ents.push_back(value.get());
		}
	}

	return ents;
}
}	// namespace enki