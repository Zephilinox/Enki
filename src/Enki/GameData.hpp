#pragma once

namespace enki
{
class Scenetree;
class NetworkManager;

//Used by Entities to access the scenetree and network manager
//Alternatively users can pass required data to a derived entity constructor
//when registering it with the scenetree
struct GameData
{
public:
	NetworkManager* network_manager = nullptr;
	Scenetree* scenetree = nullptr;
};
}	// namespace enki