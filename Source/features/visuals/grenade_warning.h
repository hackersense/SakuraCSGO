#pragma once

#include "..\..\includes.hpp"

enum class GWType {
    GW_HE_GRENADE,
    GW_MOLOTOV
};

#define GW_HIT != 1.0f
#define GW_PREDICT_INTERVAL 4
#define GW_VELOCITY_TO_MOVE_FRACTION 0.015f
#define GW_GRAVITY 0.0712f

class grenade_warning : public singleton<grenade_warning>
{
public:
    grenade_warning(player_t* entity, GWType type) {
        switch (type) {
        case GWType::GW_HE_GRENADE:
            predict_he_nade(entity, entity->m_vecOrigin(), entity->m_vecVelocity());
            distance = cheat.local()->m_vecOrigin().DistTo(end);
            safe = distance > 350;
            render_he_projectile();
            render_trail();
            break;
        case GWType::GW_MOLOTOV:
            predict_molotove_nade(entity, entity->m_vecOrigin(), entity->m_vecVelocity());
            distance = cheat.local()->m_vecOrigin().DistTo(end);
            safe = distance > 200;
            render_inc_projectile();
            render_trail();
            break;
        }
    }

private:
    float drawString(ImVec2 pos, Color color, bool center, bool outline, ImFont* pFont, const std::string& text, float size = 17)
    {
        ImVec2 textSize = pFont->CalcTextSizeA(size, FLT_MAX, 0.0f, text.c_str());

        if (center)
            pos.x -= textSize.x / 2.0f;

        if (outline) {
            ImGui::GetBackgroundDrawList()->AddText(pFont, size, ImVec2(pos.x + 1, pos.y + 1), Color(0, 0, 0, color.a()).toImColor(), text.c_str());
            ImGui::GetBackgroundDrawList()->AddText(pFont, size, ImVec2(pos.x - 1, pos.y - 1), Color(0, 0, 0, color.a()).toImColor(), text.c_str());
            ImGui::GetBackgroundDrawList()->AddText(pFont, size, ImVec2(pos.x + 1, pos.y - 1), Color(0, 0, 0, color.a()).toImColor(), text.c_str());
            ImGui::GetBackgroundDrawList()->AddText(pFont, size, ImVec2(pos.x - 1, pos.y + 1), Color(0, 0, 0, color.a()).toImColor(), text.c_str());
        }

        ImGui::GetBackgroundDrawList()->AddText(pFont, size, pos, color.toImColor(), text.c_str());

        return pos.y + textSize.y;
    }

    void predict_he_nade(player_t* nade, Vector origin, Vector velocity) {

        int log_timer = 0;
        CGameTrace trace;
        Vector current = origin;
        Vector move = velocity * GW_VELOCITY_TO_MOVE_FRACTION;
        for (unsigned int i = 0; i < 109 - nade->GetBody(); ++i)
        {
            move.z -= GW_GRAVITY;
            check_collision(current, move, trace);
            if (trace.fraction GW_HIT || log_timer % GW_PREDICT_INTERVAL == 0)
                path.push_back(current);
            current += move;
            log_timer++;
            if (trace.fraction GW_HIT)
                break;
        }
        end = current;
    }

    void predict_molotove_nade(player_t* nade, Vector origin, Vector velocity) {
        int log_timer = 0;
        CGameTrace trace;
        Vector current = origin;
        Vector move = velocity * GW_VELOCITY_TO_MOVE_FRACTION;
        for (int i = 0; i < 130 - nade->GetBody(); ++i)
        {
            move.z -= GW_GRAVITY;
            check_collision(current, move, trace);
            if (trace.fraction GW_HIT || log_timer % GW_PREDICT_INTERVAL == 0)
                path.push_back(current);
            current += move;
            log_timer++;
            if (trace.fraction GW_HIT)
                break;
        }
        end = current;
    }

    void check_collision(Vector& source, const Vector& move, CGameTrace& trace)
    {
        Vector vecAbsEnd = source;
        vecAbsEnd += move;

        m_trace()->trace_hull(source, vecAbsEnd, 0x200400B, trace);
    }

    void gradient_line(Vector Start, Vector End, Color inner, Color outer, float thickness) {
        Vector Last, Cur;
        math::world_to_screen(Start, Last);
        math::world_to_screen(End, Cur);

        if (Cur.x != 0 && Cur.y != 0 && Last.x != 0 && Last.y != 0 && Cur.x <= 1920 && Last.x <= 1920 && Cur.y <= 1080 && Last.y <= 1080) {
            ImGui::GetBackgroundDrawList()->AddQuadFilled(ImVec2(Last.x, Last.y), ImVec2(Cur.x, Cur.y), ImVec2(Cur.x + thickness, Cur.y + thickness), ImVec2(Last.x + thickness, Last.y + thickness),
                IM_COL32(255, 0, 0, 255));

            ImGui::GetBackgroundDrawList()->AddQuadFilled(ImVec2(Last.x, Last.y), ImVec2(Cur.x, Cur.y), ImVec2(Cur.x - thickness, Cur.y - thickness), ImVec2(Last.x - thickness, Last.y - thickness),
                IM_COL32(255, 0, 0, 255));

            ImGui::GetBackgroundDrawList()->AddQuadFilled(ImVec2(Last.x, Last.y), ImVec2(Cur.x, Cur.y), ImVec2(Cur.x + thickness, Cur.y - thickness), ImVec2(Last.x + thickness, Last.y - thickness),
                IM_COL32(255, 0, 0, 255));

            ImGui::GetBackgroundDrawList()->AddQuadFilled(ImVec2(Last.x, Last.y), ImVec2(Cur.x, Cur.y), ImVec2(Cur.x - thickness, Cur.y + thickness), ImVec2(Last.x - thickness, Last.y + thickness),
                IM_COL32(255, 0, 0, 255));
        }
    }

    void draw_beam(Vector S, Vector E, Color color, float W) {
        gradient_line(S, E, Color(color[0], color[1], color[2], color[3] * 0.33333333), Color(color[0], color[1], color[2], 0), W);
    }

    void render_trail()
    {
        Vector last;
        for (auto& current : path) {
            if (last.IsZero())
            {
                last = current;
                continue;
            }
            draw_beam(last, current, Color(170, 170, 255, 200), 4.3f);
            last = current;
        }
    }

    void rotate_point(Vector2D& point, Vector2D origin, bool clockwise, float angle) {
        Vector2D delta = point - origin;
        Vector2D rotated;

        if (clockwise) {
            rotated = Vector2D(delta.x * cos(angle) - delta.y * sin(angle), delta.x * sin(angle) + delta.y * cos(angle));
        }
        else {
            rotated = Vector2D(delta.x * sin(angle) - delta.y * cos(angle), delta.x * cos(angle) + delta.y * sin(angle));
        }

        point = rotated + origin;
    }

    void render_he_projectile();
    void render_inc_projectile();
    bool safe;
    float distance;
    Vector end;
    std::vector<Vector> path;
};