#pragma once

namespace enki
{
	class Scenegraph;
	class NetworkManager;

	//Used by Entities to access the scenegraph and network manager
	//Alternatively users can pass required data to a derived entity constructor
	//when registering it with the scenegraph
	struct GameData
	{
	public:
		NetworkManager* network_manager = nullptr;
		Scenegraph* scenegraph = nullptr;
	};
}