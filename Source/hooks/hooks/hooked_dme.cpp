#include "..\hooks.hpp"
#include "..\..\features\serverside\local_animations.h"
#include "..\..\features\visuals\player_esp.h"

NEWMaterial* CreateMaterial(bool lit, const std::string& material_data)
{
	static auto created = 0;
	std::string type = lit ? crypt_str("VertexLitGeneric") : crypt_str("UnlitGeneric");

	auto matname = (cheat.cheatnamelower + crypt_str("_")).c_str() + std::to_string(created);
	++created;

	auto material = m_materialsystem2()->CreateMaterial(matname.c_str(), KeyValues::FromString(type.c_str(), material_data.c_str()));
	material->IncrementReferenceCount();

	return material;
}

using DrawModelExecute_t = void(__thiscall*)(IVModelRender*, IMatRenderContext*, const DrawModelState_t&, const ModelRenderInfo_t&, matrix3x4_t*);

void __stdcall Hooks::hooked_dme(IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* bone_to_world)
{
	static auto original_fn = modelrender_hook->getOriginal<DrawModelExecute_t>(21);
	
	if (!configs.player.enabled)
		return original_fn(m_modelrender(), ctx, state, info, bone_to_world);

	if (m_engine()->IsTakingScreenshot() && configs.misc.anti_screenshot)
		return original_fn(m_modelrender(), ctx, state, info, bone_to_world);

	if (!info.pModel)
		return original_fn(m_modelrender(), ctx, state, info, bone_to_world);

	auto model_entity = static_cast<player_t *>(m_entitylist()->GetClientEntity(info.entity_index));
	auto name = m_modelinfo()->GetModelName(info.pModel);

	auto is_player = strstr(name, "models/player") && model_entity->is_alive() && (configs.player.type[ENEMY].chams[PLAYER_CHAMS_VISIBLE] || configs.player.type[TEAM].chams[PLAYER_CHAMS_VISIBLE] || configs.player.type[LOCAL].chams[PLAYER_CHAMS_VISIBLE] || configs.player.fake_chams_enabled || configs.player.backtrack_chams);
	auto is_weapon = strstr(name, "weapons/v_") && !strstr(name, "arms") && configs.esp.weapon_chams;
	auto is_arms = strstr(name, "arms") && configs.esp.arms_chams;
	auto is_sleeve = strstr(name, "sleeve") && configs.esp.arms_chams;

	if (m_modelrender()->IsForcedMaterialOverride() && !is_weapon && !is_arms && !is_sleeve)
		return original_fn(m_modelrender(), ctx, state, info, bone_to_world);

	m_renderview()->SetColorModulation(1.0f, 1.0f, 1.0f);

	if (!is_player && !is_weapon && !is_arms && !is_sleeve)
		return original_fn(m_modelrender(), ctx, state, info, bone_to_world);

	static NEWMaterial* materials[] =
	{
		CreateMaterial(true, crypt_str(R"#("VertexLitGeneric"
			{
				"$basetexture"				"vgui/white"
				"$ignorez"					"0"
				"$envmap"					" "
				"$nofog"					"1"
				"$model"					"1"
				"$nocull"					"0"
				"$selfillum"				"1"
				"$halflambert"				"1"
				"$znearer"					"0"
				"$flat"						"0"
				"$wireframe"				"0"
			}
		)#")),
		CreateMaterial(true, crypt_str(R"#("VertexLitGeneric" 
			{
				"$basetexture"				"vgui/white" 
				"$ignorez"					"0" 
				"$envmap"					"env_cubemap" 
				"$normalmapalphaenvmapmask" "1" 
				"$envmapcontrast"			"1" 
				"$nofog"					"1" 
				"$model"					"1" 
				"$nocull" 					"0" 
				"$selfillum" 				"1" 
				"$halflambert"				"1" 
				"$znearer" 					"0" 
				"$flat" 					"1"
		        "$wireframe"				"0"
			}
		)#")),
		CreateMaterial(false, crypt_str(R"#("UnlitGeneric"
			{
				"$basetexture"				"vgui/white"
				"$ignorez"					"0"
				"$envmap"					" "
				"$nofog"					"1"
				"$model"					"1"
				"$nocull"					"0"
				"$selfillum"				"1"
				"$halflambert"				"1"
				"$znearer"					"0"
				"$flat"						"1"
				"$wireframe"				"0"
			}
		)#")),
		m_materialsystem2()->FindMaterial(crypt_str("models/inventory_items/dogtags/dogtags_outline"), nullptr),
		m_materialsystem2()->FindMaterial(crypt_str("models/inventory_items/trophy_majors/crystal_clear"), nullptr),
		m_materialsystem2()->FindMaterial(crypt_str("models/inventory_items/cologne_prediction/cologne_prediction_glass"), nullptr),
		m_materialsystem2()->FindMaterial(crypt_str("dev/glow_armsrace.vmt"), nullptr),
		m_materialsystem2()->FindMaterial(crypt_str("models/inventory_items/wildfire_gold/wildfire_gold_detail"), nullptr),
		CreateMaterial(true, crypt_str(R"#("VertexLitGeneric" 
			{ 
				"$additive"					"1" 
				"$envmap"					"models/effects/cube_white" 
				"$envmaptint"				"[1 1 1]" 
				"$envmapfresnel"			"1" 
				"$envmapfresnelminmaxexp" 	"[0 1 2]" 
				"$alpha" 					"0.8" 
			}
		)#")),
		CreateMaterial(true, crypt_str(R"#("VertexLitGeneric"
		    {
		        "$basetexture"				"dev/zone_warning"
		        "$additive"					"1"
		        "$envmap"					"editor/cube_vertigo"
		        "$envmaptint"				"[0 0.5 0.55]"
		        "$envmapfresnel"			"1"
		        "$envmapfresnelminmaxexp"   "[0.00005 0.6 6]"
		        "$alpha"					"1"
   
		        Proxies
		        {
		            TextureScroll
		            {
		                "texturescrollvar"			"$baseTextureTransform"
		                "texturescrollrate"			"0.25"
		                "texturescrollangle"		"270"
		            }
		            Sine
		            {
		                "sineperiod"				"2"
		                "sinemin"					"0.1"
		                "resultVar"					"$envmapfresnelminmaxexp[1]"
		            }
		        }
		    }
		)#"))
	};

	auto called_original = false;

	if (is_player)
	{
		auto type = ENEMY;

		if (model_entity == cheat.local())
			type = LOCAL;
		else if (model_entity->m_iTeamNum() == cheat.local()->m_iTeamNum())
			type = TEAM;

		if (type == ENEMY)
		{
			auto alpha_modifier = playeresp::get().esp_alpha_fade[model_entity->EntIndex()];

			auto material = materials[configs.player.type[ENEMY].chams_type];
			auto double_material = materials[6];

			if (material && double_material && !material->IsErrorMaterial() && !double_material->IsErrorMaterial())
			{
				if (configs.player.type[ENEMY].chams[PLAYER_CHAMS_VISIBLE] && configs.player.type[ENEMY].chams[PLAYER_CHAMS_INVISIBLE])
				{
					auto alpha = (float)configs.player.backtrack_chams_color.a() / 255.0f;;

					if (configs.player.backtrack_chams)
					{
						auto backtrack_material = materials[configs.player.backtrack_chams_material];

						if (backtrack_material && !backtrack_material->IsErrorMaterial())
						{
							matrix3x4_t matrix[MAXSTUDIOBONES];

							if (util::get_backtrack_matrix(model_entity, matrix))
							{
								float backtrack_color[3] =
								{
									configs.player.backtrack_chams_color[0] / 255.0f,
									configs.player.backtrack_chams_color[1] / 255.0f,
									configs.player.backtrack_chams_color[2] / 255.0f
								};

								m_renderview()->SetBlend(alpha * alpha_modifier);
								util::color_modulate(backtrack_color, backtrack_material);

								backtrack_material->IncrementReferenceCount();
								backtrack_material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);

								m_modelrender()->ForcedMaterialOverride(backtrack_material);
								original_fn(m_modelrender(), ctx, state, info, matrix);
								m_modelrender()->ForcedMaterialOverride(nullptr);
							}
						}
					}

					alpha = (float)configs.player.type[ENEMY].xqz_color.a() / 255.0f;

					float xqz_color[3] =
					{
						configs.player.type[ENEMY].xqz_color[0] / 255.0f,
						configs.player.type[ENEMY].xqz_color[1] / 255.0f,
						configs.player.type[ENEMY].xqz_color[2] / 255.0f
					};

					m_renderview()->SetBlend(alpha * alpha_modifier);
					util::color_modulate(xqz_color, material);

					material->IncrementReferenceCount();
					material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);

					m_modelrender()->ForcedMaterialOverride(material);
					original_fn(m_modelrender(), ctx, state, info, bone_to_world);
					m_modelrender()->ForcedMaterialOverride(nullptr);
					
					alpha = (float)configs.player.type[ENEMY].chams_color.a() / 255.0f;

					float normal_color[3] =
					{
						configs.player.type[ENEMY].chams_color[0] / 255.0f,
						configs.player.type[ENEMY].chams_color[1] / 255.0f,
						configs.player.type[ENEMY].chams_color[2] / 255.0f
					};

					m_renderview()->SetBlend(alpha * alpha_modifier);
					util::color_modulate(normal_color, material);

					material->IncrementReferenceCount();
					material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

					m_modelrender()->ForcedMaterialOverride(material);
					original_fn(m_modelrender(), ctx, state, info, bone_to_world);
					m_modelrender()->ForcedMaterialOverride(nullptr);

					if (configs.player.type[ENEMY].animated_material)
					{
						auto animated_material = materials[9];
						
						if (animated_material)
						{
							alpha = (float)configs.player.type[ENEMY].animated_material_color.a() / 255.0f;

							float animated_color[3] =
							{
								configs.player.type[ENEMY].animated_material_color[0] / 255.0f,
								configs.player.type[ENEMY].animated_material_color[1] / 255.0f,
								configs.player.type[ENEMY].animated_material_color[2] / 255.0f
							};

							m_renderview()->SetBlend(alpha * alpha_modifier);
							util::color_modulate(animated_color, animated_material);

							animated_material->IncrementReferenceCount();
							animated_material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);

							m_modelrender()->ForcedMaterialOverride(animated_material);
							original_fn(m_modelrender(), ctx, state, info, bone_to_world);
							m_modelrender()->ForcedMaterialOverride(nullptr);
						}
					}

					if (configs.player.type[ENEMY].double_material && configs.player.type[ENEMY].chams_type != 6)
					{
						alpha = (float)configs.player.type[ENEMY].double_material_color.a() / 255.0f;

						float double_color[3] =
						{
							configs.player.type[ENEMY].double_material_color[0] / 255.0f,
							configs.player.type[ENEMY].double_material_color[1] / 255.0f,
							configs.player.type[ENEMY].double_material_color[2] / 255.0f
						};

						m_renderview()->SetBlend(alpha * alpha_modifier);
						util::color_modulate(double_color, double_material);

						double_material->IncrementReferenceCount();
						double_material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);

						m_modelrender()->ForcedMaterialOverride(double_material);
						original_fn(m_modelrender(), ctx, state, info, bone_to_world);
						m_modelrender()->ForcedMaterialOverride(nullptr);
					}

					called_original = true;
				}
				else if (configs.player.type[ENEMY].chams[PLAYER_CHAMS_VISIBLE])
				{
					auto alpha = (float)configs.player.backtrack_chams_color.a() / 255.0f;;

					if (configs.player.backtrack_chams)
					{
						auto backtrack_material = materials[configs.player.backtrack_chams_material];

						if (backtrack_material && !backtrack_material->IsErrorMaterial())
						{
							matrix3x4_t matrix[MAXSTUDIOBONES];

							if (util::get_backtrack_matrix(model_entity, matrix))
							{
								float backtrack_color[3] =
								{
									configs.player.backtrack_chams_color[0] / 255.0f,
									configs.player.backtrack_chams_color[1] / 255.0f,
									configs.player.backtrack_chams_color[2] / 255.0f
								};

								m_renderview()->SetBlend(alpha * alpha_modifier);
								util::color_modulate(backtrack_color, backtrack_material);

								backtrack_material->IncrementReferenceCount();
								backtrack_material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);

								m_modelrender()->ForcedMaterialOverride(backtrack_material);
								original_fn(m_modelrender(), ctx, state, info, matrix);
								m_modelrender()->ForcedMaterialOverride(nullptr);
							}
						}
					}

					alpha = (float)configs.player.type[ENEMY].chams_color.a() / 255.0f;

					float normal_color[3] =
					{
						configs.player.type[ENEMY].chams_color[0] / 255.0f,
						configs.player.type[ENEMY].chams_color[1] / 255.0f,
						configs.player.type[ENEMY].chams_color[2] / 255.0f
					};

					m_renderview()->SetBlend(alpha * alpha_modifier);
					util::color_modulate(normal_color, material);

					material->IncrementReferenceCount();
					material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

					m_modelrender()->ForcedMaterialOverride(material);
					original_fn(m_modelrender(), ctx, state, info, bone_to_world);
					m_modelrender()->ForcedMaterialOverride(nullptr);

					if (configs.player.type[ENEMY].animated_material)
					{
						auto animated_material = materials[9];

						if (animated_material)
						{
							alpha = (float)configs.player.type[ENEMY].animated_material_color.a() / 255.0f;

							float animated_color[3] =
							{
								configs.player.type[ENEMY].animated_material_color[0] / 255.0f,
								configs.player.type[ENEMY].animated_material_color[1] / 255.0f,
								configs.player.type[ENEMY].animated_material_color[2] / 255.0f
							};

							m_renderview()->SetBlend(alpha * alpha_modifier);
							util::color_modulate(animated_color, animated_material);

							animated_material->IncrementReferenceCount();
							animated_material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

							m_modelrender()->ForcedMaterialOverride(animated_material);
							original_fn(m_modelrender(), ctx, state, info, bone_to_world);
							m_modelrender()->ForcedMaterialOverride(nullptr);
						}
					}

					if (configs.player.type[ENEMY].double_material && configs.player.type[ENEMY].chams_type != 6)
					{
						alpha = (float)configs.player.type[ENEMY].double_material_color.a() / 255.0f;

						float double_color[3] =
						{
							configs.player.type[ENEMY].double_material_color[0] / 255.0f,
							configs.player.type[ENEMY].double_material_color[1] / 255.0f,
							configs.player.type[ENEMY].double_material_color[2] / 255.0f
						};

						m_renderview()->SetBlend(alpha * alpha_modifier);
						util::color_modulate(double_color, double_material);

						double_material->IncrementReferenceCount();
						double_material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

						m_modelrender()->ForcedMaterialOverride(double_material);
						original_fn(m_modelrender(), ctx, state, info, bone_to_world);
						m_modelrender()->ForcedMaterialOverride(nullptr);
					}

					called_original = true;
				}
			}

			if (!called_original)
				return original_fn(m_modelrender(), ctx, state, info, bone_to_world);
		}
		else if (type == TEAM)
		{
			auto material = materials[configs.player.type[TEAM].chams_type];
			auto double_material = materials[6];

			if (material && double_material && !material->IsErrorMaterial() && !double_material->IsErrorMaterial())
			{
				if (configs.player.type[TEAM].chams[PLAYER_CHAMS_VISIBLE] && configs.player.type[TEAM].chams[PLAYER_CHAMS_INVISIBLE])
				{
					auto alpha = (float)configs.player.type[TEAM].xqz_color.a() / 255.0f;

					float xqz_color[3] =
					{
						configs.player.type[TEAM].xqz_color[0] / 255.0f,
						configs.player.type[TEAM].xqz_color[1] / 255.0f,
						configs.player.type[TEAM].xqz_color[2] / 255.0f
					};

					m_renderview()->SetBlend(alpha);
					util::color_modulate(xqz_color, material);

					material->IncrementReferenceCount();
					material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);

					m_modelrender()->ForcedMaterialOverride(material);
					original_fn(m_modelrender(), ctx, state, info, bone_to_world);
					m_modelrender()->ForcedMaterialOverride(nullptr);

					alpha = (float)configs.player.type[TEAM].chams_color.a() / 255.0f;

					float normal_color[3] =
					{
						configs.player.type[TEAM].chams_color[0] / 255.0f,
						configs.player.type[TEAM].chams_color[1] / 255.0f,
						configs.player.type[TEAM].chams_color[2] / 255.0f
					};

					m_renderview()->SetBlend(alpha);
					util::color_modulate(normal_color, material);

					material->IncrementReferenceCount();
					material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

					m_modelrender()->ForcedMaterialOverride(material);
					original_fn(m_modelrender(), ctx, state, info, bone_to_world);
					m_modelrender()->ForcedMaterialOverride(nullptr);

					if (configs.player.type[TEAM].animated_material)
					{
						auto animated_material = materials[9];

						if (animated_material)
						{
							alpha = (float)configs.player.type[TEAM].animated_material_color.a() / 255.0f;

							float animated_color[3] =
							{
								configs.player.type[TEAM].animated_material_color[0] / 255.0f,
								configs.player.type[TEAM].animated_material_color[1] / 255.0f,
								configs.player.type[TEAM].animated_material_color[2] / 255.0f
							};

							m_renderview()->SetBlend(alpha);
							util::color_modulate(animated_color, animated_material);

							animated_material->IncrementReferenceCount();
							animated_material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);

							m_modelrender()->ForcedMaterialOverride(animated_material);
							original_fn(m_modelrender(), ctx, state, info, bone_to_world);
							m_modelrender()->ForcedMaterialOverride(nullptr);
						}
					}

					if (configs.player.type[TEAM].double_material && configs.player.type[TEAM].chams_type != 6)
					{
						alpha = (float)configs.player.type[TEAM].double_material_color.a() / 255.0f;

						float double_color[3] =
						{
							configs.player.type[TEAM].double_material_color[0] / 255.0f,
							configs.player.type[TEAM].double_material_color[1] / 255.0f,
							configs.player.type[TEAM].double_material_color[2] / 255.0f
						};

						m_renderview()->SetBlend(alpha);
						util::color_modulate(double_color, double_material);

						double_material->IncrementReferenceCount();
						double_material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);

						m_modelrender()->ForcedMaterialOverride(double_material);
						original_fn(m_modelrender(), ctx, state, info, bone_to_world);
						m_modelrender()->ForcedMaterialOverride(nullptr);
					}

					called_original = true;
				}
				else if (configs.player.type[TEAM].chams[PLAYER_CHAMS_VISIBLE])
				{
					auto alpha = (float)configs.player.type[TEAM].chams_color.a() / 255.0f;

					float normal_color[3] =
					{
						configs.player.type[TEAM].chams_color[0] / 255.0f,
						configs.player.type[TEAM].chams_color[1] / 255.0f,
						configs.player.type[TEAM].chams_color[2] / 255.0f
					};

					m_renderview()->SetBlend(alpha);
					util::color_modulate(normal_color, material);

					material->IncrementReferenceCount();
					material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

					m_modelrender()->ForcedMaterialOverride(material);
					original_fn(m_modelrender(), ctx, state, info, bone_to_world);
					m_modelrender()->ForcedMaterialOverride(nullptr);

					if (configs.player.type[TEAM].animated_material)
					{
						auto animated_material = materials[9];

						if (animated_material)
						{
							alpha = (float)configs.player.type[TEAM].animated_material_color.a() / 255.0f;

							float animated_color[3] =
							{
								configs.player.type[TEAM].animated_material_color[0] / 255.0f,
								configs.player.type[TEAM].animated_material_color[1] / 255.0f,
								configs.player.type[TEAM].animated_material_color[2] / 255.0f
							};

							m_renderview()->SetBlend(alpha);
							util::color_modulate(animated_color, animated_material);

							animated_material->IncrementReferenceCount();
							animated_material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

							m_modelrender()->ForcedMaterialOverride(animated_material);
							original_fn(m_modelrender(), ctx, state, info, bone_to_world);
							m_modelrender()->ForcedMaterialOverride(nullptr);
						}
					}

					if (configs.player.type[TEAM].double_material && configs.player.type[TEAM].chams_type != 6)
					{
						alpha = (float)configs.player.type[TEAM].double_material_color.a() / 255.0f;

						float double_color[3] =
						{
							configs.player.type[TEAM].double_material_color[0] / 255.0f,
							configs.player.type[TEAM].double_material_color[1] / 255.0f,
							configs.player.type[TEAM].double_material_color[2] / 255.0f
						};

						m_renderview()->SetBlend(alpha);
						util::color_modulate(double_color, double_material);

						double_material->IncrementReferenceCount();
						double_material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

						m_modelrender()->ForcedMaterialOverride(double_material);
						original_fn(m_modelrender(), ctx, state, info, bone_to_world);
						m_modelrender()->ForcedMaterialOverride(nullptr);
					}

					called_original = true;
				}
			}

			if (!called_original)
				return original_fn(m_modelrender(), ctx, state, info, bone_to_world);
		}
		else if (m_input()->m_bCameraInThirdPerson)
		{
			auto alpha_modifier = 1.0f;

			if (configs.player.transparency_in_scope && cheat.globals.scoped)
				alpha_modifier = configs.player.transparency_in_scope_amount;

			auto material = materials[configs.player.type[LOCAL].chams_type];
			auto double_material = materials[6];

			if (material && double_material && !material->IsErrorMaterial() && !double_material->IsErrorMaterial())
			{
				if (configs.player.type[LOCAL].chams[PLAYER_CHAMS_VISIBLE] && configs.player.type[LOCAL].chams[PLAYER_CHAMS_INVISIBLE])
				{
					auto alpha = (float)configs.player.type[LOCAL].xqz_color.a() / 255.0f * alpha_modifier;

					float xqz_color[3] =
					{
						configs.player.type[LOCAL].xqz_color[0] / 255.0f,
						configs.player.type[LOCAL].xqz_color[1] / 255.0f,
						configs.player.type[LOCAL].xqz_color[2] / 255.0f
					};

					m_renderview()->SetBlend(alpha);
					util::color_modulate(xqz_color, material);

					material->IncrementReferenceCount();
					material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);

					m_modelrender()->ForcedMaterialOverride(material);
					original_fn(m_modelrender(), ctx, state, info, bone_to_world);
					m_modelrender()->ForcedMaterialOverride(nullptr);

					alpha = (float)configs.player.type[LOCAL].chams_color.a() / 255.0f * alpha_modifier;

					float normal_color[3] =
					{
						configs.player.type[LOCAL].chams_color[0] / 255.0f,
						configs.player.type[LOCAL].chams_color[1] / 255.0f,
						configs.player.type[LOCAL].chams_color[2] / 255.0f
					};

					m_renderview()->SetBlend(alpha);
					util::color_modulate(normal_color, material);

					material->IncrementReferenceCount();
					material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

					m_modelrender()->ForcedMaterialOverride(material);
					original_fn(m_modelrender(), ctx, state, info, bone_to_world);
					m_modelrender()->ForcedMaterialOverride(nullptr);

					if (configs.player.type[LOCAL].animated_material)
					{
						auto animated_material = materials[9];

						if (animated_material)
						{
							auto alpha = (float)configs.player.type[LOCAL].animated_material_color.a() / 255.0f;

							float animated_color[3] =
							{
								configs.player.type[LOCAL].animated_material_color[0] / 255.0f,
								configs.player.type[LOCAL].animated_material_color[1] / 255.0f,
								configs.player.type[LOCAL].animated_material_color[2] / 255.0f
							};

							m_renderview()->SetBlend(alpha);
							util::color_modulate(animated_color, animated_material);

							animated_material->IncrementReferenceCount();
							animated_material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);

							m_modelrender()->ForcedMaterialOverride(animated_material);
							original_fn(m_modelrender(), ctx, state, info, bone_to_world);
							m_modelrender()->ForcedMaterialOverride(nullptr);
						}
					}

					if (configs.player.type[LOCAL].double_material && configs.player.type[LOCAL].chams_type != 6)
					{
						alpha = (float)configs.player.type[LOCAL].double_material_color.a() / 255.0f * alpha_modifier;

						float double_color[3] =
						{
							configs.player.type[LOCAL].double_material_color[0] / 255.0f,
							configs.player.type[LOCAL].double_material_color[1] / 255.0f,
							configs.player.type[LOCAL].double_material_color[2] / 255.0f
						};

						m_renderview()->SetBlend(alpha);
						util::color_modulate(double_color, double_material);

						double_material->IncrementReferenceCount();
						double_material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);

						m_modelrender()->ForcedMaterialOverride(double_material);
						original_fn(m_modelrender(), ctx, state, info, bone_to_world);
						m_modelrender()->ForcedMaterialOverride(nullptr);
					}

					called_original = true;
				}
				else if (configs.player.type[LOCAL].chams[PLAYER_CHAMS_VISIBLE])
				{
					auto alpha = (float)configs.player.type[LOCAL].chams_color.a() / 255.0f * alpha_modifier;

					float normal_color[3] =
					{
						configs.player.type[LOCAL].chams_color[0] / 255.0f,
						configs.player.type[LOCAL].chams_color[1] / 255.0f,
						configs.player.type[LOCAL].chams_color[2] / 255.0f
					};

					m_renderview()->SetBlend(alpha);
					util::color_modulate(normal_color, material);

					material->IncrementReferenceCount();
					material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

					m_modelrender()->ForcedMaterialOverride(material);
					original_fn(m_modelrender(), ctx, state, info, bone_to_world);
					m_modelrender()->ForcedMaterialOverride(nullptr);

					if (configs.player.type[LOCAL].animated_material)
					{
						auto animated_material = materials[9];

						if (animated_material)
						{
							alpha = (float)configs.player.type[LOCAL].animated_material_color.a() / 255.0f * alpha_modifier;

							float animated_color[3] =
							{
								configs.player.type[LOCAL].animated_material_color[0] / 255.0f,
								configs.player.type[LOCAL].animated_material_color[1] / 255.0f,
								configs.player.type[LOCAL].animated_material_color[2] / 255.0f
							};

							m_renderview()->SetBlend(alpha);
							util::color_modulate(animated_color, animated_material);

							animated_material->IncrementReferenceCount();
							animated_material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

							m_modelrender()->ForcedMaterialOverride(animated_material);
							original_fn(m_modelrender(), ctx, state, info, bone_to_world);
							m_modelrender()->ForcedMaterialOverride(nullptr);
						}
					}

					if (configs.player.type[LOCAL].double_material && configs.player.type[LOCAL].chams_type != 6)
					{
						alpha = (float)configs.player.type[LOCAL].double_material_color.a() / 255.0f * alpha_modifier;

						float double_color[3] =
						{
							configs.player.type[LOCAL].double_material_color[0] / 255.0f,
							configs.player.type[LOCAL].double_material_color[1] / 255.0f,
							configs.player.type[LOCAL].double_material_color[2] / 255.0f
						};

						m_renderview()->SetBlend(alpha);
						util::color_modulate(double_color, double_material);

						double_material->IncrementReferenceCount();
						double_material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

						m_modelrender()->ForcedMaterialOverride(double_material);
						original_fn(m_modelrender(), ctx, state, info, bone_to_world);
						m_modelrender()->ForcedMaterialOverride(nullptr);
					}

					called_original = true;
				}
			}

			if (!called_original && configs.player.layered)
			{
				m_renderview()->SetBlend(alpha_modifier);
				m_renderview()->SetColorModulation(1.0f, 1.0f, 1.0f);

				original_fn(m_modelrender(), ctx, state, info, bone_to_world);
			}

			if (configs.player.fake_chams_enabled && !antiaim::get().freeze_check && (configs.antiaim.enabled || configs.fakelag.enabled))
			{
				if (!local_animations::get().local_data.visualize_lag)
				{
					for (auto& i : cheat.globals.fake_matrix)
					{
						i[0][3] += info.origin.x;
						i[1][3] += info.origin.y;
						i[2][3] += info.origin.z;
					}
				}

				auto alpha = (float)configs.player.fake_chams_color.a() / 255.0f;
				material = materials[configs.player.fake_chams_type];

				float fake_color[3] =
				{
					configs.player.fake_chams_color[0] / 255.0f,
					configs.player.fake_chams_color[1] / 255.0f,
					configs.player.fake_chams_color[2] / 255.0f
				};

				m_renderview()->SetBlend(alpha);
				util::color_modulate(fake_color, material);

				material->IncrementReferenceCount();
				material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

				m_modelrender()->ForcedMaterialOverride(material);
				original_fn(m_modelrender(), ctx, state, info, cheat.globals.fake_matrix);
				m_modelrender()->ForcedMaterialOverride(nullptr);

				if (configs.player.fake_animated_material)
				{
					auto animated_material = materials[9];

					if (animated_material)
					{
						alpha = (float)configs.player.fake_animated_material_color.a() / 255.0f;

						float animated_color[3] =
						{
							configs.player.fake_animated_material_color[0] / 255.0f,
							configs.player.fake_animated_material_color[1] / 255.0f,
							configs.player.fake_animated_material_color[2] / 255.0f
						};

						m_renderview()->SetBlend(alpha);
						util::color_modulate(animated_color, animated_material);

						animated_material->IncrementReferenceCount();
						animated_material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

						m_modelrender()->ForcedMaterialOverride(animated_material);
						original_fn(m_modelrender(), ctx, state, info, cheat.globals.fake_matrix);
						m_modelrender()->ForcedMaterialOverride(nullptr);
					}
				}

				if (configs.player.fake_double_material && configs.player.fake_chams_type != 6 && double_material)
				{
					alpha = (float)configs.player.fake_double_material_color.a() / 255.0f;

					float double_color[3] =
					{
						configs.player.fake_double_material_color[0] / 255.0f,
						configs.player.fake_double_material_color[1] / 255.0f,
						configs.player.fake_double_material_color[2] / 255.0f
					};

					m_renderview()->SetBlend(alpha);
					util::color_modulate(double_color, double_material);

					double_material->IncrementReferenceCount();
					double_material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

					m_modelrender()->ForcedMaterialOverride(double_material);
					original_fn(m_modelrender(), ctx, state, info, cheat.globals.fake_matrix);
					m_modelrender()->ForcedMaterialOverride(nullptr);
				}

				if (!local_animations::get().local_data.visualize_lag)
				{
					for (auto& i : cheat.globals.fake_matrix)
					{
						i[0][3] -= info.origin.x;
						i[1][3] -= info.origin.y;
						i[2][3] -= info.origin.z;
					}
				}
			}

			if (!called_original && !configs.player.layered)
			{
				m_renderview()->SetBlend(alpha_modifier);
				m_renderview()->SetColorModulation(1.0f, 1.0f, 1.0f);

				original_fn(m_modelrender(), ctx, state, info, bone_to_world);
			}
		}
	}
	else if (is_weapon)
	{
		auto alpha = (float)configs.esp.weapon_chams_color.a() / 255.0f;

		auto material = materials[configs.esp.weapon_chams_type];
		auto double_material = materials[6];

		if (material && double_material && !material->IsErrorMaterial() && !double_material->IsErrorMaterial())
		{
			float weapon_color[3] =
			{
				configs.esp.weapon_chams_color[0] / 255.0f,
				configs.esp.weapon_chams_color[1] / 255.0f,
				configs.esp.weapon_chams_color[2] / 255.0f
			};

			m_renderview()->SetBlend(alpha);
			util::color_modulate(weapon_color, material);

			material->IncrementReferenceCount();
			material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

			m_modelrender()->ForcedMaterialOverride(material);
			original_fn(m_modelrender(), ctx, state, info, bone_to_world);
			m_modelrender()->ForcedMaterialOverride(nullptr);

			if (configs.esp.weapon_animated_material)
			{
				auto animated_material = materials[9];

				if (animated_material)
				{
					auto alpha = (float)configs.esp.weapon_animated_material_color.a() / 255.0f;

					float animated_color[3] =
					{
						configs.esp.weapon_animated_material_color[0] / 255.0f,
						configs.esp.weapon_animated_material_color[1] / 255.0f,
						configs.esp.weapon_animated_material_color[2] / 255.0f
					};

					m_renderview()->SetBlend(alpha);
					util::color_modulate(animated_color, animated_material);

					animated_material->IncrementReferenceCount();
					animated_material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

					m_modelrender()->ForcedMaterialOverride(animated_material);
					original_fn(m_modelrender(), ctx, state, info, bone_to_world);
					m_modelrender()->ForcedMaterialOverride(nullptr);
				}
			}

			if (configs.esp.weapon_double_material && configs.esp.weapon_chams_type != 6)
			{
				alpha = (float)configs.esp.weapon_double_material_color.a() / 255.0f;

				float double_color[3] =
				{
					configs.esp.weapon_double_material_color[0] / 255.0f,
					configs.esp.weapon_double_material_color[1] / 255.0f,
					configs.esp.weapon_double_material_color[2] / 255.0f
				};

				m_renderview()->SetBlend(alpha);
				util::color_modulate(double_color, double_material);

				double_material->IncrementReferenceCount();
				double_material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

				m_modelrender()->ForcedMaterialOverride(double_material);
				original_fn(m_modelrender(), ctx, state, info, bone_to_world);
				m_modelrender()->ForcedMaterialOverride(nullptr);
			}

			called_original = true;
		}

		if (!called_original)
			return original_fn(m_modelrender(), ctx, state, info, bone_to_world);
	}
	else if (is_arms || is_sleeve)
	{
		auto alpha = (float)configs.esp.arms_chams_color.a() / 255.0f;

		auto material = materials[configs.esp.arms_chams_type];
		auto double_material = materials[6];

		if (material && double_material && !material->IsErrorMaterial() && !double_material->IsErrorMaterial())
		{
			float arms_color[3] =
			{
				configs.esp.arms_chams_color[0] / 255.0f,
				configs.esp.arms_chams_color[1] / 255.0f,
				configs.esp.arms_chams_color[2] / 255.0f
			};

			m_renderview()->SetBlend(alpha);
			util::color_modulate(arms_color, material);

			material->IncrementReferenceCount();
			material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

			m_modelrender()->ForcedMaterialOverride(material);
			original_fn(m_modelrender(), ctx, state, info, bone_to_world);
			m_modelrender()->ForcedMaterialOverride(nullptr);

			if (configs.esp.arms_animated_material)
			{
				auto animated_material = materials[9];

				if (animated_material)
				{
					auto alpha = (float)configs.esp.arms_animated_material_color.a() / 255.0f;

					float animated_color[3] =
					{
						configs.esp.arms_animated_material_color[0] / 255.0f,
						configs.esp.arms_animated_material_color[1] / 255.0f,
						configs.esp.arms_animated_material_color[2] / 255.0f
					};

					m_renderview()->SetBlend(alpha);
					util::color_modulate(animated_color, animated_material);

					animated_material->IncrementReferenceCount();
					animated_material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

					m_modelrender()->ForcedMaterialOverride(animated_material);
					original_fn(m_modelrender(), ctx, state, info, bone_to_world);
					m_modelrender()->ForcedMaterialOverride(nullptr);
				}
			}

			if (configs.esp.arms_double_material && configs.esp.arms_chams_type != 6)
			{
				alpha = (float)configs.esp.arms_double_material_color.a() / 255.0f;

				float double_color[3] =
				{
					configs.esp.arms_double_material_color[0] / 255.0f,
					configs.esp.arms_double_material_color[1] / 255.0f,
					configs.esp.arms_double_material_color[2] / 255.0f
				};

				m_renderview()->SetBlend(alpha);
				util::color_modulate(double_color, double_material);

				double_material->IncrementReferenceCount();
				double_material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

				m_modelrender()->ForcedMaterialOverride(double_material);
				original_fn(m_modelrender(), ctx, state, info, bone_to_world);
				m_modelrender()->ForcedMaterialOverride(nullptr);
			}

			called_original = true;
		}

		if (!called_original)
			return original_fn(m_modelrender(), ctx, state, info, bone_to_world);
	}
}