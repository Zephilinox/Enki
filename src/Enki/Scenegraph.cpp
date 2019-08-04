#include "Scenegraph.hpp"

//STD
#include <experimental/map>

//LIBS
#include <spdlog/sinks/stdout_color_sinks.h>

//SELF
#include "Networking/ServerHost.hpp"
#include "Networking/ClientHost.hpp"
#include "Networking/ClientStandard.hpp"

namespace enki
{
	Scenegraph::Scenegraph(GameData* game_data)
		: game_data(game_data)
	{
		console = spdlog::get("Enki");
		if (console == nullptr)
		{
			spdlog::stdout_color_mt("Enki");
			console = spdlog::get("Enki");
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
			mc1 = game_data->network_manager->server->on_packet_received.connect([this](Packet p)
			{
				auto server = game_data->network_manager->server.get();

				if (p.getHeader().type == ENTITY_CREATION)
				{
					auto info = p.read<EntityInfo>();
					auto spawnInfo = p.read<Packet>();
					info.ownerID = p.info.senderID;
					createNetworkedEntity(info, spawnInfo);
				}
				if (p.getHeader().type == ENTITY_CREATION_ON_CONNECTION)
				{
					console->error("Server received ENTITY_CREATION_ON_CONNECTION");
				}
				else if (p.getHeader().type == CONNECTED)
				{
					sendAllNetworkedEntitiesToClient(p.info.senderID);
				}
				else if (p.getHeader().type == ENTITY_UPDATE)
				{
					//Don't send entity updates back to the sender
					server->sendPacketToAllExceptOneClient(p.info.senderID, 0, &p);
				}
				else if (p.getHeader().type == ENTITY_RPC)
				{
					auto info = p.read<EntityInfo>();
					auto name = p.read<std::string>();
					auto rpctype = rpc_man.getRPCType(info.type, name);

					if (entityExists(info.ID))
					{
						auto ent = getEntity(info.ID);
						if (info == ent->info)
						{
							if (rpctype == Local)
							{
								console->error(
									"Received a request for a "
									"Local Entity RPC call from {} "
									"but local calls shouldn't be "
									"sent over the network", p.info.senderID);
							}
							else if (rpctype == Master)
							{
								if (info.ownerID == p.info.senderID)
								{
									console->error(
										"Received a request for a "
										"Master Entity RPC call from {} "
										"but the sender is the master of {}",
										p.info.senderID, info);
								}
								else
								{
									//only send packet to master
									server->sendPacketToOneClient(info.ownerID, 0, &p);
								}
							}
							else if (rpctype == Remote || rpctype == RemoteAndLocal)
							{
								if (info.ownerID == p.info.senderID)
								{
									//only send packets to non-owners, which must be all except sender
									server->sendPacketToAllExceptOneClient(
										p.info.senderID, 0, &p);
								}
								else
								{
									console->error(
										"Received request for Remote "
										"or RemoteAndLocal RPC call from {} "
										"but the sent entity's owner ID "
										"doesn't match the sender's ID. "
										"Entity is {}", info);
								}
							}
							else if (rpctype == MasterAndRemote || rpctype == All)
							{
								//send to everyone else
								server->sendPacketToAllExceptOneClient(
									p.info.senderID, 0, &p);
							}
							else
							{
								console->error(
									"Received request for an Entity RPC "
									"from {} but the RPC type is invalid",
									p.info.senderID);
							}
						}
					}
				}
				else if (p.getHeader().type == ENTITY_DELETION)
				{
					auto entID = p.read<EntityID>();
					if (entityExists(entID))
					{
						auto ent = getEntity(entID);
						if (ent->info.ownerID == p.info.senderID)
						{
							game_data->network_manager->server->sendPacketToAllClients(0, &p);
						}
						else
						{
							console->error(
								"Received request to delete entity {} "
								"but the sender {} isn't the owner");
						}
					}
					else
					{
						console->error(
							"Received request to delete entity {} "
							"but it doesn't exist");
					}
				}
			});
		}

		if (game_data->network_manager->client)
		{
			mc2 = game_data->network_manager->client->on_packet_received.connect([this](Packet p)
			{
				if (p.getHeader().type == ENTITY_CREATION)
				{
					auto info = p.read<EntityInfo>();
					auto spawnInfo = p.read<Packet>();
					createEntity(info, spawnInfo);
				}
				else if (p.getHeader().type == ENTITY_CREATION_ON_CONNECTION)
				{
					auto info = p.read<EntityInfo>();
					auto ent = createEntity(info);
					ent->deserializeOnConnection(p);
				}
				else if (p.getHeader().type == ENTITY_UPDATE)
				{
					auto info = p.read<EntityInfo>();
					if (entityExists(info.ID))
					{
						auto ent = getEntity(info.ID);
						if (info == ent->info)
						{
							ent->deserializeOnTick(p);
						}
						else
						{
							console->error(
								"Received entity update with "
								"invalid info.\n\t{}\n\tVS\n\t{}"
								"\n\tSender ID = {}, Client ID = {}",
								info, ent->info,
								p.info.senderID,
								game_data->network_manager->client->getID());
						}
					}
					else
					{
						console->error(
							"Received entity update for nonexistant entity."
							"\n\t{}", info);
					}
				}
				else if (p.getHeader().type == ENTITY_RPC)
				{
					auto info = p.read<EntityInfo>();
					if (entityExists(info.ID))
					{
						auto ent = getEntity(info.ID);
						if (info == ent->info)
						{
							p.resetReadPosition();
							rpc_man.receive(p, ent);
						}
						else
						{
							console->error(
								"Received an RPC packet from {} for an entity "
								"that does not match our version."
								"\nTheirs: \t{}"
								"\nOurs: \t{}", p.info.senderID, info, ent->info);
						}
					}
					else
					{
						console->error(
							"Received an RPC packet from {} "
							"for an entity that does not exist."
							"\n\t{}", p.info.senderID, info);
					}
				}
				else if (p.getHeader().type == ENTITY_DELETION)
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
							entID);
					}
				}
			});

			mc3 = game_data->network_manager->on_network_tick.connect([this]()
			{
				total_network_ticks++;

				enki::Packet p({ enki::PacketType::ENTITY_UPDATE });

				for (auto& ent : entities)
				{
					//Serialize our entities based on their specific tick rates
					if (ent.second->isOwner() &&
						ent.second->network_tick_rate > 0 &&
						total_network_ticks % ent.second->network_tick_rate == 0)
					{
						p.clear();
						p << ent.second->info;
						ent.second->serializeOnTick(p);
						game_data->network_manager->client->sendPacket(0, &p);
					}
				}
			});
		}
	}

	void Scenegraph::input(sf::Event& e)
	{
		for (auto& ent : entities)
		{
			if (ent.second->info.parentID == 0)
			{
				ent.second->input(e);
				inputHierarchy(e, ent.second->info.ID);
			}
		}
	}

	void Scenegraph::update(float dt)
	{
		for (auto& ent : entities)
		{
			if (ent.second->info.parentID == 0)
			{
				ent.second->update(dt);
				updateHierarchy(dt, ent.second->info.ID);
			}
		}

		std::experimental::erase_if(entities, [](const auto& ent)
		{
			return ent.second->remove;
		});
	}

	void Scenegraph::draw(Renderer* renderer)
	{
		for (const auto& ent : entities)
		{
			if (ent.second->info.parentID == 0)
			{
				ent.second->draw(renderer);
				drawHierarchy(renderer, ent.second->info.ID);
			}
		}
	}

	void Scenegraph::registerEntity(const std::string& type, BuilderFunction builder)
	{
		builders[type] = std::move(builder);
	}

	Entity* Scenegraph::createEntity(EntityInfo info)
	{
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
		if (info.name.empty() || info.type.empty())
		{
			console->error("Invalid info when creating entity.\n\t{}", info);
			return nullptr;
		}

		if (!builders.count(info.type))
		{
			console->error("Tried to create entity without registering it first.\n\t{}", info);
			return nullptr;
		}

		if (info.ID == 0)
		{
			info.ID = localID--;
		}

		//info gets assigned to the entity here through being passed to the Entity base class constructor
		entities[info.ID] = builders.at(info.type)(info);
		entities[info.ID]->onSpawn(spawnInfo);

		return entities[info.ID].get();
	}

	void Scenegraph::createNetworkedEntity(EntityInfo info, Packet& spawnInfo)
	{
		auto net_man = game_data->network_manager;

		if (info.name.empty() || info.type.empty())
		{
			console->error("Invalid info when creating networked entity.\n\t{}", info);
			return;
		}

		if (!builders.count(info.type))
		{
			console->error("Tried to create entity without registering it first.\n\t{}", info);
			return;
		}

		if (net_man->server && info.ID == 0)
		{
			info.ID = ID++;
		}

		for (auto& child_info : entities_child_types[info.type])
		{
			createNetworkedEntity({ child_info.type, child_info.name, 0, info.ownerID, info.ID }, child_info.spawnInfo);
		}

		if (info.ownerID == 0 && network_ready)
		{
			if (net_man->client)
			{
				info.ownerID = net_man->client->getID();
			}
			else
			{
				info.ownerID = 1;
			}
		}

		if (network_ready)
		{
			Packet p({ PacketType::ENTITY_CREATION });
			p << info;
			p << spawnInfo;

			if (net_man->server)
			{
				console->info("Creating networked entity as the server to all clients.\n\t{}", info);
				net_man->server->sendPacketToAllClients(0, &p);
			}
			else if (net_man->client)
			{
				console->info("Creating networked entity as the client to the server.\n\t{}", info);
				net_man->client->sendPacket(0, &p);
			}
			else
			{
				console->error("Tried to create networked entity when network isn't running");
			}
		}
		else
		{
			console->error("Tried to create networked entity when scenegraph isn't network ready");
		}
	}

	void Scenegraph::inputHierarchy(sf::Event& e, EntityID parentID)
	{
		for (auto& ent : entities)
		{
			if (ent.second->info.parentID == parentID)
			{
				ent.second->input(e);
				inputHierarchy(e, ent.second->info.ID);
			}
		}
	}

	void Scenegraph::updateHierarchy(float dt, EntityID parentID)
	{
		for (auto& ent : entities)
		{
			if (ent.second->info.parentID == parentID)
			{
				ent.second->update(dt);
				updateHierarchy(dt, ent.second->info.ID);
			}
		}
	}

	void Scenegraph::drawHierarchy(enki::Renderer* renderer, EntityID parentID)
	{
		for (auto& ent : entities)
		{
			if (ent.second->info.parentID == parentID)
			{
				ent.second->draw(renderer);
				drawHierarchy(renderer, ent.second->info.ID);
			}
		}
	}

	void Scenegraph::sendAllNetworkedEntitiesToClient(ClientID client_id)
	{
		if (network_ready && game_data->network_manager->server)
		{
			for (auto i = entities.rbegin(); i != entities.rend(); ++i)
			{
				auto& ent = *i;
				EntityInfo info = ent.second->info;

				if (info.name.empty() || info.type.empty())
				{
					console->error("Invalid info when sending on connection of client {} for entity.\n\t{}", client_id, info);
				}
				else
				{
					console->info("Sending networked entity to client {}.\n\t{}", client_id, info);
				}

				{
					Packet p({ PacketType::ENTITY_CREATION_ON_CONNECTION });
					p << info;
					ent.second->serializeOnConnection(p);
					game_data->network_manager->server->sendPacketToOneClient(client_id, 0, &p);
				}
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
		if (entities[entityID]->info.ID < 0)
		{
			entities[entityID]->remove = true;
		}
		else if (network_ready)
		{
			Packet p({ ENTITY_DELETION });
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

	std::vector<Entity*> Scenegraph::findEntitiesByType(const std::string& type) const
	{
		std::vector<Entity*> ents;

		for (const auto& ent : entities)
		{
			if (ent.second->info.type == type)
			{
				ents.push_back(ent.second.get());
			}
		}

		return ents;
	}

	std::vector<Entity*> Scenegraph::findEntitiesByName(const std::string& name) const
	{
		std::vector<Entity*> ents;

		for (const auto& ent : entities)
		{
			if (ent.second->info.name == name)
			{
				ents.push_back(ent.second.get());
			}
		}

		return ents;
	}

	std::vector<Entity*> Scenegraph::findEntitiesByOwner(ClientID owner) const
	{
		std::vector<Entity*> ents;

		for (const auto& ent : entities)
		{
			if (ent.second->info.ownerID == owner)
			{
				ents.push_back(ent.second.get());
			}
		}

		return ents;
	}

	std::vector<Entity*> Scenegraph::findEntitiesByParent(EntityID parent) const
	{
		std::vector<Entity*> ents;

		for (const auto& ent : entities)
		{
			if (ent.second->info.parentID == parent)
			{
				ents.push_back(ent.second.get());
			}
		}

		return ents;
	}

	std::vector<Entity*> Scenegraph::findEntitiesByPredicate(const std::function<bool(const Entity&)>& predicate) const
	{
		std::vector<Entity*> ents;

		for (const auto& ent : entities)
		{
			if (predicate(*ent.second.get()))
			{
				ents.push_back(ent.second.get());
			}
		}

		return ents;
	}
}