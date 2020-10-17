#include "ScenetreeGUI.hpp"

namespace enki
{

ScenetreeGUI::ScenetreeGUI(EntityInfo info, Scenetree* scenetree)
	: Entity(std::move(info))
	, scenetree(scenetree)
{
	
}

void ScenetreeGUI::input(Event& e)
{
}

void treenode(Scenetree* scenetree, const Entity* ent)
{
	std::string pretty_id = prettyID(ent->info.ID);
	std::string combined = ent->info.name + " " + pretty_id;

	if (ent->isLocal())
		ImGui::PushStyleColor(ImGuiCol_Text, {1, 0.7, 0.3, 1});
	else
		ImGui::PushStyleColor(ImGuiCol_Text, {0.5, 1, 0.5, 1});
	
	const bool node = ImGui::TreeNode(combined.c_str());

	ImGui::PopStyleColor();
	
	if (node)
	{
		std::string type_hash_to_string = "Type: " + prettyHash(ent->info.type);
		std::string id = "ID: " + pretty_id;
		std::string local = "Local: " + std::string(ent->isLocal() ? "true" : "false");
		std::string owner = "Owner: " + prettyClientID(scenetree->getNetworkManager()->client->getID(), ent->info.ownerID);

		ImGui::Text(type_hash_to_string.c_str());
		ImGui::Text(id.c_str());
		ImGui::Text(owner.c_str());
		ImGui::Text(local.c_str());

		auto data = ent->serializeToStrings();
		for (const auto& [name, value] : data)
		{
			combined = name + ": " + value;
			ImGui::Text(combined.c_str());
		}
		
		if (!ent->info.childIDs.empty())
			ImGui::Separator();

		for (const auto& child_id : ent->info.childIDs)
		{
			const auto* child_ent = scenetree->findEntity(child_id);
			if (!child_ent)
				continue;

			treenode(scenetree, child_ent);
		}
		
		ImGui::TreePop();
	}
}

void ScenetreeGUI::update(float dt)
{
	if (!show)
		return;
	
	if (!ImGui::Begin("Scenetree", &show))
	{
		ImGui::End();
		return;
	}

	auto root_ent_ids = scenetree->getRootEntitiesIDs();

	for (const auto root_ent_id : root_ent_ids)
	{
		const auto* ent = scenetree->findEntity(root_ent_id);
		if (!ent)
			continue;

		treenode(scenetree, ent);
	}

	ImGui::SetWindowSize({200, 200}, ImGuiCond_FirstUseEver);
	ImGui::SetWindowPos({300, 300}, ImGuiCond_FirstUseEver);
	ImGui::End();
}

}