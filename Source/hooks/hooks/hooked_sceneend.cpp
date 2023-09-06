#include "..\hooks.hpp"

using SceneEnd_t = void(__thiscall*)(void*);

void __fastcall Hooks::hooked_sceneend(void* ecx, void* edx)
{
	static auto original_fn = renderview_hook->getOriginal<SceneEnd_t>(9);

	if (!cheat.local())
		return original_fn(ecx);

	if (!configs.esp.type[ENEMY].ragdoll_chams && !configs.esp.type[TEAM].ragdoll_chams)
		return original_fn(ecx);

	const auto HighestEntityIndex = m_entitylist()->GetHighestEntityIndex();
	for (auto i = 1; i <= HighestEntityIndex; i++)
	{
		auto e = reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(i));

		if (!e) 
			continue;

		if (e->is_alive())
			continue;

		auto client_class = e->GetClientClass();

		if (!client_class)
			continue;

		if (client_class->m_ClassID != CCSRagdoll)
			continue;

		auto type = ENEMY;

		if (e->m_iTeamNum() == cheat.local()->m_iTeamNum())
			type = TEAM;
		
		if (!configs.esp.type[type].ragdoll_chams)
			continue;

		static NEWMaterial* chams_materials[] =
		{
			m_materialsystem2()->FindMaterial((cheat.cheatnamelower + crypt_str("_chams")).c_str(), nullptr),
			m_materialsystem2()->FindMaterial((cheat.cheatnamelower + crypt_str("_metallic")).c_str(), nullptr),
			m_materialsystem2()->FindMaterial((cheat.cheatnamelower + crypt_str("_flat")).c_str(), nullptr),
			m_materialsystem2()->FindMaterial(crypt_str("models/inventory_items/dogtags/dogtags_outline"), nullptr),
			m_materialsystem2()->FindMaterial(crypt_str("models/inventory_items/trophy_majors/crystal_clear"), nullptr),
			m_materialsystem2()->FindMaterial(crypt_str("models/inventory_items/cologne_prediction/cologne_prediction_glass"), nullptr),
			m_materialsystem2()->FindMaterial(crypt_str("dev/glow_armsrace.vmt"), nullptr),
			m_materialsystem2()->FindMaterial(crypt_str("models/inventory_items/wildfire_gold/wildfire_gold_detail"), nullptr),
			m_materialsystem2()->FindMaterial((cheat.cheatnamelower + crypt_str("_glow")).c_str(), nullptr)
		};

		auto material = chams_materials[configs.esp.type[type].ragdoll_chams_material];

		if (material && !material->IsErrorMaterial())
		{
			auto alpha = (float)configs.esp.type[type].ragdoll_chams_color.a() / 255.0f;

			float ragdoll_color[3] =
			{
				configs.esp.type[type].ragdoll_chams_color[0] / 255.0f,
				configs.esp.type[type].ragdoll_chams_color[1] / 255.0f,
				configs.esp.type[type].ragdoll_chams_color[2] / 255.0f
			};

			m_renderview()->SetBlend(alpha);
			util::color_modulate(ragdoll_color, material);

			material->IncrementReferenceCount();
			material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

			m_modelrender()->ForcedMaterialOverride(material);
			e->DrawModel(0x1, 255);
			m_modelrender()->ForcedMaterialOverride(nullptr);
		}
	}

	return original_fn(ecx);
}
