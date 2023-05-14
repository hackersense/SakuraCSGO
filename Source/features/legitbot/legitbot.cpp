#include "legitbot.h"
#include "backtrack.h"

void legitbot::run(CUserCmd* cmd)
{
    if (!configs.legitbot.enabled || configs.ragebot.enabled)
        return;

    if (!m_engine()->IsActiveApp())
        return;

    if (!key_binds::get().get_key_bind_state(key_bind_features::LEGIT_BOT))
        return;

    auto server_time = util::server_time();

    if (cheat.local()->m_flNextAttack() > server_time || cheat.local()->m_bIsDefusing() || cheat.local()->m_bWaitForNoAttack())
        return;

    const auto& activeWeapon = cheat.globals.weapon;
    if (!activeWeapon || !activeWeapon->m_iClip() || activeWeapon->is_non_aim())
        return;

    if (!(cheat.local()->m_fFlags() & FL_ONGROUND) && configs.legitbot.jump_check)
        return;

    if (!configs.legitbot.weapon[cheat.globals.current_weapon].between_shots && activeWeapon->m_flNextPrimaryAttack() > server_time)
        return;

    if (!configs.legitbot.ignore_flash && cheat.local()->m_flFlashDuration() > 0)
        return;

    if (cmd->m_buttons & IN_ATTACK || configs.legitbot.weapon[cheat.globals.current_weapon].aimlock)
    {
        if ((int)(activeWeapon->get_inaccuracy() + activeWeapon->get_spread()) > configs.legitbot.weapon[cheat.globals.current_weapon].maxAimInaccuracy)
            return;

        if (configs.legitbot.sniper_scope_only && activeWeapon->is_sniper() && !cheat.local()->m_bIsScoped())
            return;

        auto bestFov = configs.legitbot.weapon[cheat.globals.current_weapon].fov;
        Vector bestTarget{ };
        player_t* bestEntity{ };
        const auto localPlayerEyePosition = cheat.local()->get_shoot_position();

        static auto weapon_recoil_scale = m_cvar()->FindVar(crypt_str("weapon_recoil_scale"));

        const auto aimPunch = activeWeapon->requires_recoil_control() ? cheat.local()->m_aimPunchAngle() * weapon_recoil_scale->GetFloat() : ZERO;

        if (configs.legitbot.weapon[cheat.globals.current_weapon].standaloneRCS && (configs.legitbot.weapon[cheat.globals.current_weapon].recoilControlHorizontal > 0.f || configs.legitbot.weapon[cheat.globals.current_weapon].recoilControlVertical > 0.f) && activeWeapon->requires_recoil_control())
        {
            static Vector lastAimPunch{ };
            if (cheat.local()->m_iShotsFired() > configs.legitbot.weapon[cheat.globals.current_weapon].shotsFired)
            {
                if (configs.legitbot.weapon[cheat.globals.current_weapon].randomRCS)
                    math::random_seed(*memory->predictionRandomSeed);

                Vector currentPunch;
                if (configs.legitbot.weapon[cheat.globals.current_weapon].silentRCS)
                    currentPunch = aimPunch;
                else
                    currentPunch = lastAimPunch - aimPunch;

                currentPunch.x *= configs.legitbot.weapon[cheat.globals.current_weapon].randomRCS ? math::random_float(configs.legitbot.weapon[cheat.globals.current_weapon].recoilControlHorizontal, 1.f) : configs.legitbot.weapon[cheat.globals.current_weapon].recoilControlHorizontal;
                currentPunch.y *= configs.legitbot.weapon[cheat.globals.current_weapon].randomRCS ? math::random_float(configs.legitbot.weapon[cheat.globals.current_weapon].recoilControlVertical, 1.f) : configs.legitbot.weapon[cheat.globals.current_weapon].recoilControlVertical;

                if (configs.legitbot.weapon[cheat.globals.current_weapon].silentRCS)
                    cmd->m_viewangles -= currentPunch;
                else
                    cmd->m_viewangles += currentPunch;
            }
            if (!configs.legitbot.weapon[cheat.globals.current_weapon].silentRCS)
                m_engine()->SetViewAngles(cmd->m_viewangles);
            lastAimPunch = aimPunch;
        }

        for (auto i = 1; i < m_globals()->m_maxclients; i++)
        {
            auto entity = reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(i));
            if (!entity->valid(!configs.legitbot.friendly_fire))
                continue;

            for (auto bone : { 8, 4, 3, 7, 6, 5 })
            {
                auto bonePosition = entity->bone_position(configs.legitbot.weapon[cheat.globals.current_weapon].hitboxes > 1 ? 10 - configs.legitbot.weapon[cheat.globals.current_weapon].hitboxes : bone);
                const auto angle = math::calculate_relative_angle(localPlayerEyePosition, bonePosition, cmd->m_viewangles + aimPunch);

                const auto fov = std::hypot(angle.x, angle.y);
                if (fov > bestFov)
                    continue;

                if (!configs.legitbot.ignore_smoke && memory->lineGoesThroughSmoke(localPlayerEyePosition, bonePosition, 1))
                    continue;

                int minimum_damage;

                if (configs.legitbot.weapon[cheat.globals.current_weapon].min_damage > 100)
                    minimum_damage = (configs.legitbot.weapon[cheat.globals.current_weapon].min_damage - 100) + entity->m_iHealth();
                else
                    minimum_damage = min(configs.legitbot.weapon[cheat.globals.current_weapon].min_damage, 1);

                auto info = autowall::get().wall_penetration(localPlayerEyePosition, bonePosition, entity);

                if (!info.valid)
                    continue;

                if ((!info.visible && !configs.legitbot.autowall) || (info.damage < minimum_damage))
                    continue;

                if (fov < bestFov) {
                    bestFov = fov;
                    bestTarget = bonePosition;
                    bestEntity = entity;
                }
            }
        }

        if (bestEntity && bestTarget.notNull()) {
            static Vector lastAngles{ cmd->m_viewangles };
            static int lastCommand{ };

            if (lastCommand == cmd->m_command_number - 1 && lastAngles.notNull() && configs.legitbot.weapon[cheat.globals.current_weapon].silent)
                cmd->m_viewangles = lastAngles;

            auto angle = math::calculate_relative_angle(localPlayerEyePosition, bestTarget, cmd->m_viewangles + aimPunch);
            bool clamped{ false };

            if (std::abs(angle.x) > 255.f || std::abs(angle.y) > 255.f) {
                angle.x = std::clamp(angle.x, -255.f, 255.f);
                angle.y = std::clamp(angle.y, -255.f, 255.f);
                clamped = true;
            }

            if (configs.legitbot.smooth_mode != 0)
            {
                if (configs.legitbot.smooth_mode == 2)
                {
                    auto smooth_value = configs.legitbot.weapon[cheat.globals.current_weapon].smooth;
                    smooth_value = math::clamp((smooth_value + (math::random_float(-0.2f, 0.2f) * smooth_value)), configs.legitbot.weapon[cheat.globals.current_weapon].smooth, 13.f);
                    angle /= smooth_value;
                }
                else
                    angle /= configs.legitbot.weapon[cheat.globals.current_weapon].smooth;
            }
            cmd->m_viewangles += angle;
            
            if (!configs.legitbot.weapon[cheat.globals.current_weapon].silent)
                m_engine()->SetViewAngles(cmd->m_viewangles);

            if (configs.legitbot.weapon[cheat.globals.current_weapon].autostop)
                cmd->m_forwardmove = cmd->m_sidemove = 0;

            if (configs.legitbot.autoscope && activeWeapon->m_flNextPrimaryAttack() <= server_time && activeWeapon->is_sniper() && !cheat.local()->m_bIsScoped())
                cmd->m_buttons |= IN_ZOOM;

            if (clamped)
                cmd->m_buttons &= ~IN_ATTACK;

            if (clamped || (configs.legitbot.weapon[cheat.globals.current_weapon].smooth) > 1.0f)
                lastAngles = cmd->m_viewangles;
            else
                lastAngles = ZERO;

            lastCommand = cmd->m_command_number;
        }
    }
}
