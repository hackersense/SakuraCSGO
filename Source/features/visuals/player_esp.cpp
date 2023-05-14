#include "player_esp.h"
#include "..\misc\misc.h"
#include "..\ragebot\ragebot.h"
#include "..\menu.h"
#include "..\..\sdk\BBData.h"

void playeresp::render()
{
	if (!configs.player.enabled)
		return;

	if (!m_engine()->IsInGame())
		return;

	static auto alpha = 1.0f;
	//c_dormant_esp::get().start();

	if (configs.player.arrows && cheat.local()->is_alive())
	{
		static auto switch_alpha = false;

		if (alpha <= 0.0f || alpha >= 1.0f)
			switch_alpha = !switch_alpha;

		alpha += switch_alpha ? 2.0f * m_globals()->m_frametime : -2.0f * m_globals()->m_frametime;
		alpha = math::clamp(alpha, 0.0f, 1.0f);
	}

	//for (auto i = 1; i < 65; i++)
	for (int i = 1; i < m_globals()->m_maxclients; i++)
	{
		//auto e = reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(i));

		//if (!e->valid(false, false))
		//	continue;

		auto boxInfo = BBData::getBox(i);

		if (!boxInfo.valid)
			continue;

		auto e = boxInfo.player;

		if (!e || boxInfo.absOrigin == ZERO)
			continue;

		type = ENEMY;

		if (e == cheat.local())
			type = LOCAL;
		else if (e->m_iTeamNum() == cheat.local()->m_iTeamNum())
			type = TEAM;

		if (type == LOCAL && !m_input()->m_bCameraInThirdPerson)
			continue;

		/*
		auto valid_dormant = false;
		auto backup_flags = e->m_fFlags();
		auto& backup_origin = e->GetAbsOrigin();

		if (e->IsDormant())
			valid_dormant = c_dormant_esp::get().adjust_sound(e);
		else
		{
			health[i] = e->m_iHealth();
			c_dormant_esp::get().m_cSoundPlayers[i].reset(true, e->GetAbsOrigin(), e->m_fFlags());
		}

		if (radar_base && hud_radar && e->IsDormant() && e->m_iTeamNum() != cheat.local()->m_iTeamNum() && e->m_bSpotted())
			health[i] = hud_radar->radar_info[i].health;

		if (!health[i])
		{
			if (e->IsDormant())
			{
				e->m_fFlags() = backup_flags;
				e->set_abs_origin(backup_origin);
			}

			continue;
		}

		auto fast = 2.5f * m_globals()->m_frametime;
		auto slow = 0.25f * m_globals()->m_frametime;

		if (e->IsDormant())
		{
			auto& origin = e->GetAbsOrigin();

			if (origin == ZERO)
				esp_alpha_fade[i] = 0.0f;
			else if (!valid_dormant && esp_alpha_fade[i] > 0.0f)
				esp_alpha_fade[i] -= slow;
			else if (valid_dormant && esp_alpha_fade[i] < 1.0f)
				esp_alpha_fade[i] += fast;
		}
		else if (esp_alpha_fade[i] < 1.0f)
			esp_alpha_fade[i] += fast;

		esp_alpha_fade[i] = math::clamp(esp_alpha_fade[i], 0.0f, 1.0f);

		*/

		//auto boxInfo = BBData::getBox(i);

		if (configs.player.type[type].skeleton && boxInfo.skeletonValid)
		{
			auto color = e->IsDormant() ? Color(130, 130, 130, 130) : configs.player.type[type].skeleton_color;
			color.SetAlpha(min(255.0f * esp_alpha_fade[i], color.a()));

			/*
			auto records = &player_records[i];
			if (!records->empty() && cheat.local()->is_alive())
			{
				auto record = &records->front();
				if (record->matrixes_data.main)
					draw_skeleton(e, color, record->matrixes_data.main);
				else
					draw_skeleton(e, color, e->m_CachedBoneData().Base());
			}
			else*/
			if (cheat.local()->is_alive() || cheat.local()->m_iObserverMode() != ObserverType_t::OBS_MODE_IN_EYE || cheat.local()->m_hObserverTarget() != e)
				draw_skeleton(boxInfo, color); // , e->m_CachedBoneData().Base()
			/*
						auto records = &player_records[i];

			if (!records->empty() && cheat.local()->is_alive() && !e->IsDormant())
			{
				auto record = &records->front();

				draw_skeleton(e, color, record->matrixes_data.main);
				draw_skeleton(e, Color::Red, record->matrixes_data.zero);
				draw_skeleton(e, Color::Green, record->matrixes_data.first);
				draw_skeleton(e, Color::Blue, record->matrixes_data.second);
			}
			else
				draw_skeleton(e, color, e->m_CachedBoneData().Base());
				// draw all skeleton
			*/
		}

		//if (util::get_bbox(e, box, true))
		if (boxInfo.valid)
		{
			auto box = boxInfo.box;

			auto hpbox = hp_info[i];

			if (hpbox.hp == -1)
				hpbox.hp = math::clamp(health[i], 0, 100);
			else
			{
				auto hp = math::clamp(health[i], 0, 100);

				if (hp != hpbox.hp)
				{
					if (hpbox.hp > hp)
					{
						if (hpbox.hp_difference_time)
							hpbox.hp_difference += hpbox.hp - hp;
						else
							hpbox.hp_difference = hpbox.hp - hp;

						hpbox.hp_difference_time = m_globals()->m_curtime;
					}
					else
					{
						hpbox.hp_difference = 0;
						hpbox.hp_difference_time = 0.0f;
					}

					hpbox.hp = hp;
				}

				if (m_globals()->m_curtime - hpbox.hp_difference_time > 0.2f && hpbox.hp_difference)
				{
					auto difference_factor = 4.0f * m_globals()->m_frametime * hpbox.hp_difference;

					hpbox.hp_difference -= difference_factor;
					hpbox.hp_difference = math::clamp(hpbox.hp_difference, 0, 100);

					if (!hpbox.hp_difference)
						hpbox.hp_difference_time = 0.0f;
				}
			}

			draw_box(e, box);
			draw_name(e, box);
			draw_health(e, box, hpbox);
			draw_weapon(e, box, draw_ammobar(e, box));
			draw_flags(e, box);
		}

		if (type == ENEMY || type == TEAM)
		{
			draw_lines(e, boxInfo.absOrigin);

			if (type == ENEMY)
			{
				if (configs.player.arrows && cheat.local()->is_alive())
				{
					auto color = configs.player.arrows_color;
					color.SetAlpha((int)(min(255.0f * esp_alpha_fade[i] * alpha, color.a())));

					if (e->IsDormant())
						color = Color(130, 130, 130, (int)(255.0f * esp_alpha_fade[i]));

					misc::get().pov_arrows(boxInfo.renderOrigin, color);
				}

				if (!e->IsDormant())
					draw_multi_points(e);
			}
		}

		//if (e->IsDormant()) {
		//	e->m_fFlags() = backup_flags;
		//	e->set_abs_origin(backup_origin);
		//}
	}
}

void playeresp::draw_skeleton(BoxInfo info, Color color) // , player_t* e, matrix3x4_t matrix[MAXSTUDIOBONES]
{
	/*
	for (int i = 0; i < MAXSTUDIOBONES; i++) {
		auto child = info.child[i];
		auto parent = info.parent[i];

		if (child.IsZero() || parent.IsZero()) // BAD SKELETON
			return;
	}
	*/
	for (int i = 0; i < MAXSTUDIOBONES; i++) {
		auto child = info.child[i];
		auto parent = info.parent[i];

		auto schild = ZERO;
		auto sparent = ZERO;

		if (!child.IsZero() && !parent.IsZero())
			if (math::world_to_screen_pixel_aligned(parent, sparent) && math::world_to_screen_pixel_aligned(child, schild))
				imRender->DrawLine(sparent.x, sparent.y, schild.x, schild.y, color, 1.f);
	}
	/*
	auto model = e->GetModel();

	if (!model)
		return;

	auto studio_model = m_modelinfo()->GetStudioModel(model);

	if (!studio_model)
		return;

	auto get_bone_position = [&](int bone) -> Vector
	{
		return Vector(matrix[bone][0][3], matrix[bone][1][3], matrix[bone][2][3]);
	};

	auto upper_direction = get_bone_position(7) - get_bone_position(6);
	auto breast_bone = get_bone_position(6) + upper_direction * 0.5f;

	for (auto i = 0; i < studio_model->numbones; i++)
	{
		auto bone = studio_model->pBone(i);

		if (!bone)
			continue;

		if (bone->parent == -1)
			continue;

		if (!(bone->flags & BONE_USED_BY_HITBOX))
			continue;

		auto child = get_bone_position(i);
		auto parent = get_bone_position(bone->parent);

		auto delta_child = child - breast_bone;
		auto delta_parent = parent - breast_bone;

		if (delta_parent.Length() < 9.0f && delta_child.Length() < 9.0f)
			parent = breast_bone;

		if (i == 5)
			child = breast_bone;

		if (fabs(delta_child.z) < 5.0f && delta_parent.Length() < 5.0f && delta_child.Length() < 5.0f || i == 6)
			continue;

		auto schild = ZERO;
		auto sparent = ZERO;

		if (math::world_to_screen(child, schild) && math::world_to_screen(parent, sparent))
			imRender->DrawLine(schild.x, schild.y, sparent.x, sparent.y, color);
	}
	*/
}

void playeresp::draw_box(player_t* m_entity, const Box& box)
{
	if (!configs.player.type[type].box)
		return;

	auto alpha = 255.0f * esp_alpha_fade[m_entity->EntIndex()];
	auto outline_alpha = (int)(alpha * 0.6f);

	Color outline_color
	{
		0,
		0,
		0,
		outline_alpha
	};

	auto color = m_entity->IsDormant() ? Color(130, 130, 130, 130) : configs.player.type[type].box_color;
	color.SetAlpha(min(alpha, color.a()));

	imRender->Rect(box.x - 1, box.y - 1, box.w + 2, box.h + 2, outline_color, 0, -1);
	imRender->Rect(box.x, box.y, box.w, box.h, color, 0, -1);
	imRender->Rect(box.x + 1, box.y + 1, box.w - 2, box.h - 2, outline_color, 0, -1);
}

void playeresp::draw_health(player_t* m_entity, const Box& box, const HPInfo& hpinfo)
{
	if (!configs.player.type[type].health)
		return;

	auto alpha = (int)(255.0f * esp_alpha_fade[m_entity->EntIndex()]);

	auto text_color = m_entity->IsDormant() ? Color(130, 130, 130, alpha) : Color(255, 255, 255, alpha);
	auto back_color = Color(0, 0, 0, (int)(alpha * 0.6f));
	auto color = m_entity->IsDormant() ? Color(130, 130, 130) : Color(150, (int)min(255.0f, hpinfo.hp * 225.0f / 100.0f), 0);
	auto hp_effect_color = Color(215, 20, 20, alpha);

	if (configs.player.type[type].custom_health_color)
		color = m_entity->IsDormant() ? Color(130, 130, 130) : configs.player.type[type].health_color;

	color.SetAlpha(alpha);
	imRender->Rect(box.x - 6, box.y, 4, box.h, back_color);

	Box n_box =
	{
		box.x - 5,
		box.y,
		2,
		box.h
	};

	auto bar_height = (int)((float)hpinfo.hp * (float)n_box.h / 100.0f);
	auto offset = n_box.h - bar_height;

	imRender->FilledRect(n_box.x, n_box.y - 1, 2, n_box.h + 2, back_color);
	imRender->FilledRect(n_box.x, n_box.y + offset, 2, bar_height, color);

	auto height = n_box.h - hpinfo.hp * n_box.h / 100;

	if (hpinfo.hp_difference)
		imRender->FilledRect(n_box.x, n_box.y + height - hpinfo.hp_difference * n_box.h / 100.f, 2, hpinfo.hp_difference * n_box.h / 100.f, hp_effect_color);

	if (hpinfo.hp < 100)
		imRender->DrawString(n_box.x + 1, n_box.y + offset, text_color, IFONT_CENTERED_X | IFONT_CENTERED_Y, skeet_menu.f2, std::to_string(hpinfo.hp).c_str());
}

bool playeresp::draw_ammobar(player_t* m_entity, const Box& box)
{
	if (!m_entity->is_alive())
		return false;

	if (!configs.player.type[type].ammo)
		return false;

	auto weapon = m_entity->m_hActiveWeapon().Get();

	if (weapon->is_non_aim())
		return false;

	auto ammo = weapon->m_iClip();

	auto alpha = (int)(255.0f * esp_alpha_fade[m_entity->EntIndex()]);
	auto outline_alpha = (int)(alpha * 0.7f);
	auto inner_back_alpha = (int)(alpha * 0.6f);

	Color outline_color = Color(0, 0, 0, outline_alpha);

	Color inner_back_color = Color(0, 0, 0, inner_back_alpha);

	auto text_color = m_entity->IsDormant() ? Color(130, 130, 130, alpha) : Color(255, 255, 255, alpha);
	auto color = m_entity->IsDormant() ? Color(130, 130, 130, 130) : configs.player.type[type].ammobar_color;

	color.SetAlpha(min(alpha, color.a()));

	Box n_box =
	{
		box.x,
		box.y + box.h + 3,
		box.w + 2,
		2
	};

	auto weapon_info = weapon->get_weapon_info();

	if (!weapon_info)
		return false;

	auto bar_width = ammo * box.w / weapon_info->iMaxClip1;
	auto reloading = false;

	auto animlayer = m_entity->get_animlayers()[ANIMATION_LAYER_WEAPON_ACTION];

	if (animlayer.m_iSequence)
	{
		auto activity = m_entity->sequence_activity(animlayer.m_iSequence);

		reloading = activity == ACT_CSGO_RELOAD && animlayer.m_flWeight;

		if (reloading && animlayer.m_flCycle < 1.0f)
			bar_width = animlayer.m_flCycle * box.w;
	}

	imRender->FilledRect(n_box.x - 1, n_box.y - 1, n_box.w, 4, inner_back_color);
	imRender->FilledRect(n_box.x, n_box.y, bar_width, 2, color);

	if (weapon->m_iClip() != weapon_info->iMaxClip1 && !reloading)
		imRender->DrawString(n_box.x + bar_width, n_box.y + 1, text_color, IFONT_CENTERED_X | IFONT_CENTERED_Y, skeet_menu.f2, std::to_string(ammo).c_str());

	return true;
}

void playeresp::draw_name(player_t* m_entity, const Box& box)
{
	if (!configs.player.type[type].name)
		return;

	static auto sanitize = [](char* name) -> std::wstring
	{
		name[127] = '\0';

		std::wstring tmp(StringUtils::stringToWString(std::string(name))); // Fix wide character "?"

		if (tmp.length() > 20)
		{
			tmp.erase(20, tmp.length() - 20);
			tmp.append(L"...");
		}

		return tmp;
	};

	player_info_t player_info;

	if (m_engine()->GetPlayerInfo(m_entity->EntIndex(), &player_info))
	{
		auto name = sanitize(player_info.name);

		auto color = m_entity->IsDormant() ? Color(130, 130, 130, 130) : configs.player.type[type].name_color;
		color.SetAlpha(min(255.0f * esp_alpha_fade[m_entity->EntIndex()], color.a()));

		if (unicodeFont.fonts.chinese)
			imRender->DrawString(box.x + box.w / 2.f, box.y - 18, color, IFONT_CENTERED_X | IFONT_OUTLINE, unicodeFont.fonts.chinese, StringUtils::wstringToString(name));
		else
			imRender->DrawString(box.x + box.w / 2.f, box.y - 13, color, IFONT_CENTERED_X | IFONT_OUTLINE, skeet_menu.f2, StringUtils::wstringToString(name));
	}
}

void playeresp::draw_weapon(player_t* m_entity, const Box& box, bool space)
{
	if (!configs.player.type[type].weapon[WEAPON_ICON] && !configs.player.type[type].weapon[WEAPON_TEXT])
		return;

	auto weapon = m_entity->m_hActiveWeapon().Get();

	if (!weapon)
		return;

	auto pos = box.y + box.h + 2;

	if (space)
		pos += 5;

	auto color = m_entity->IsDormant() ? Color(130, 130, 130, 130) : configs.player.type[type].weapon_color;
	color.SetAlpha(min(255.0f * esp_alpha_fade[m_entity->EntIndex()], color.a()));

	if (configs.player.type[type].weapon[WEAPON_TEXT])
	{
		imRender->DrawString(box.x + box.w / 2.f, pos, color, IFONT_CENTERED_X | IFONT_OUTLINE, skeet_menu.f2, weapon->get_name().c_str());
		pos += 11;
	}

	if (configs.player.type[type].weapon[WEAPON_ICON])
	{
		//if (weapon->m_iItemDefinitionIndex() == WEAPON_KNIFE)
		//	imRender->text(fonts[KNIFES], box.x + box.w / 2, pos, color, IFONT_CENTERED_X, "z");
		//else if (weapon->m_iItemDefinitionIndex() == WEAPON_KNIFE_T)
		//	imRender->text(fonts[KNIFES], box.x + box.w / 2, pos, color, IFONT_CENTERED_X, "W");
		//else
		imRender->DrawString(box.x + box.w / 2.f, pos, color, IFONT_CENTERED_X | IFONT_OUTLINE, skeet_menu.weapon_font, weapon->get_icon());
	}
}

void playeresp::draw_flags(player_t* e, const Box& box)
{
	auto weapon = e->m_hActiveWeapon().Get();

	if (!weapon)
		return;

	auto _x = box.x + box.w + 3, _y = box.y - 3;

	if (configs.player.type[type].flags[FLAGS_MONEY])
	{
		auto color = e->IsDormant() ? Color(130, 130, 130, 130) : Color(170, 190, 80);
		color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);

		imRender->DrawString(_x, _y, color, IFONT_OUTLINE, skeet_menu.f2, "%i$", e->m_iAccount());
		_y += 8;
	}

	if (configs.player.type[type].flags[FLAGS_ARMOR])
	{
		auto color = e->IsDormant() ? Color(130, 130, 130, 130) : Color(240, 240, 240);
		color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);

		auto kevlar = e->m_ArmorValue() > 0;
		auto helmet = e->m_bHasHelmet();

		std::string text;

		if (helmet && kevlar)
			text = "HK";
		else if (kevlar)
			text = "K";

		if (kevlar)
		{
			imRender->DrawString(_x, _y, color, IFONT_OUTLINE, skeet_menu.f2, text.c_str());
			_y += 8;
		}
	}

	if (configs.player.type[type].flags[FLAGS_KIT] && e->m_bHasDefuser())
	{
		auto color = e->IsDormant() ? Color(130, 130, 130, 130) : Color(240, 240, 240);
		color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);

		imRender->DrawString(_x, _y, color, IFONT_OUTLINE, skeet_menu.f2, "KIT");
		_y += 8;
	}

	if (configs.player.type[type].flags[FLAGS_SCOPED])
	{
		auto scoped = e->m_bIsScoped();

		if (e == cheat.local())
			scoped = cheat.globals.scoped;

		if (scoped)
		{
			auto color = e->IsDormant() ? Color(130, 130, 130, 130) : Color(30, 120, 235);
			color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);

			imRender->DrawString(_x, _y, color, IFONT_OUTLINE, skeet_menu.f2, "SCOPED");
			_y += 8;
		}
	}

	if (configs.player.type[type].flags[FLAGS_FAKEDUCKING])
	{
		auto animstate = e->get_animation_state();

		if (animstate)
		{
			auto fakeducking = [&]() -> bool
			{
				static auto stored_tick = 0;
				static int crouched_ticks[65];

				if (animstate->m_flDuckAmount)
				{
					if (animstate->m_flDuckAmount < 0.9f && animstate->m_flDuckAmount > 0.5f)
					{
						if (stored_tick != m_globals()->m_tickcount)
						{
							crouched_ticks[e->EntIndex()]++;
							stored_tick = m_globals()->m_tickcount;
						}

						return crouched_ticks[e->EntIndex()] > 16;
					}
					else
						crouched_ticks[e->EntIndex()] = 0;
				}

				return false;
			};

			if (fakeducking() && e->m_fFlags() & FL_ONGROUND && !animstate->m_bInHitGroundAnimation)
			{
				auto color = e->IsDormant() ? Color(130, 130, 130, 130) : Color(215, 20, 20);
				color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);

				imRender->DrawString(_x, _y, color, IFONT_OUTLINE, skeet_menu.f2, "FD");
				_y += 8;
			}
		}
	}

	if (configs.player.type[type].flags[FLAGS_VULNERABLE] && weapon->is_non_aim())
	{
		auto color = e->IsDormant() ? Color(130, 130, 130, 130) : Color(205, 180, 110);
		color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);

		imRender->DrawString(_x, _y, color, IFONT_OUTLINE, skeet_menu.f2, "VULNERABLE");
		_y += 8;
	}

	if (configs.player.type[type].flags[FLAGS_PING])
	{
		player_info_t player_info;
		m_engine()->GetPlayerInfo(e->EntIndex(), &player_info);

		if (player_info.fakeplayer)
		{
			auto color = e->IsDormant() ? Color(130, 130, 130, 130) : Color(240, 240, 240);
			color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);

			imRender->DrawString(_x, _y, color, IFONT_OUTLINE, skeet_menu.f2, "BOT");
			_y += 8;
		}
		else
		{
			auto latency = math::clamp(m_playerresource()->GetPing(e->EntIndex()), 0, 1000);
			std::string delay = std::to_string(latency) + " MS";

			auto green_factor = (int)math::clamp(255.0f - (float)latency * 225.0f / 200.0f, 0.0f, 255.0f);

			auto color = e->IsDormant() ? Color(130, 130, 130, 130) : Color(150, green_factor, 0);
			color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);

			imRender->DrawString(_x, _y, color, IFONT_OUTLINE, skeet_menu.f2, delay.c_str());
			_y += 8;
		}
	}

	if (configs.player.type[type].flags[FLAGS_C4] && e->EntIndex() == cheat.globals.bomb_carrier)
	{
		auto color = e->IsDormant() ? Color(130, 130, 130, 130) : Color(163, 49, 93);
		color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);

		imRender->DrawString(_x, _y, color, IFONT_OUTLINE, skeet_menu.f2, "C4");
		_y += 8;
	}
}

void playeresp::draw_lines(player_t* e, Vector origin)
{
	if (!configs.player.type[type].snap_lines)
		return;

	if (!cheat.local()->is_alive())
		return;

	static int width, height;
	m_engine()->GetScreenSize(width, height);

	Vector angle;

	if (!math::world_to_screen_pixel_aligned(origin, angle))
		return;

	auto color = e->IsDormant() ? Color(130, 130, 130, 130) : configs.player.type[type].snap_lines_color;
	color.SetAlpha(min(255.0f * esp_alpha_fade[e->EntIndex()], color.a()));

	imRender->DrawLine(width / 2.f, height, angle.x, angle.y, color);
}

void playeresp::draw_multi_points(player_t* e)
{
	if (!configs.ragebot.enabled)
		return;

	if (!configs.player.show_multi_points)
		return;

	if (!cheat.local()->is_alive())
		return;

	if (cheat.local()->get_move_type() == MOVETYPE_NOCLIP)
		return;

	if (cheat.globals.current_weapon == -1)
		return;

	auto weapon = cheat.local()->m_hActiveWeapon().Get();

	if (weapon->is_non_aim())
		return;

	auto records = &player_records[e->EntIndex()];

	if (records->empty())
		return;

	auto record = &records->front();

	if (!record->valid(false))
		return;

	std::vector<scan_point> points;
	auto hitboxes = ragebot::get().get_hitboxes(record);

	for (auto& hitbox : hitboxes)
	{
		auto current_points = ragebot::get().get_points(record, hitbox, false);

		for (auto& point : current_points)
			points.emplace_back(point);
	}

	for (auto& point : points)
	{
		if (points.empty())
			break;

		if (point.hitbox == HITBOX_HEAD)
			continue;

		for (auto it = points.begin(); it != points.end(); ++it)
		{
			if (point.point == it->point)
				continue;

			auto first_angle = math::calculate_angle(cheat.globals.eye_pos, point.point);
			auto second_angle = math::calculate_angle(cheat.globals.eye_pos, it->point);

			auto distance = cheat.globals.eye_pos.DistTo(point.point);
			auto fov = math::fast_sin(DEG2RAD(math::get_fov(first_angle, second_angle))) * distance;

			if (fov < 5.0f)
			{
				points.erase(it);
				break;
			}
		}
	}

	if (points.empty())
		return;

	for (auto& point : points)
	{
		Vector screen;

		if (!math::world_to_screen(point.point, screen))
			continue;

		imRender->FilledRect(screen.x - 1, screen.y - 1, 3, 3, configs.player.show_multi_points_color);
		imRender->Rect(screen.x - 2, screen.y - 2, 5, 5, Color::Black);
	}
}