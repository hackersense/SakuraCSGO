#include "grenade_warning.h"
#include "../menu.h"

void grenade_warning::render_he_projectile() {
    Vector Screen;
    if (math::world_to_screen(end, Screen))
    {
        imRender->CircleFilled(Screen.x, Screen.y, 26 - 10 + 5, safe ? Color(170, 170, 255, 190) : Color(255, 44, 20, 255), 25);
        imRender->CircleFilled(Screen.x, Screen.y, 26 - 10, Color(2, 9, 46, 200), 25);
        drawString(ImVec2(Screen.x + 1, Screen.y - 1), Color(255, 255, 255, 255), true, false, skeet_menu.weapon_font, "j");
    }
    else if (distance <= 450) {
        Vector dir;
        m_engine()->GetViewAngles(dir);

        float ViewAngle = dir.y;

        if (ViewAngle < 0.f)
        {
            ViewAngle += 360.f;
        }
        ViewAngle = DEG2RAD(ViewAngle);

        auto TowardsAngle = math::calculate_angle(cheat.local()->m_vecOrigin(), end);
        TowardsAngle.Normalized();

        if (TowardsAngle.y < 0.f) {
            TowardsAngle.y += 360.f;
        }
        TowardsAngle.y = DEG2RAD(TowardsAngle.y);
        TowardsAngle.y -= ViewAngle;
        Vector2D Screen2;
        Screen2 = Vector2D(1920 / 2 - 340, 1080 / 2 - 340);
        rotate_point(Screen2, Vector2D(1920 / 2, 1080 / 2), false, TowardsAngle.y);

        imRender->CircleFilled(Screen2.x, Screen2.y, 26 - 10 + 5, safe ? Color(170, 170, 255, 190) : Color(255, 44, 20, 255), 35);
        imRender->CircleFilled(Screen2.x, Screen2.y, 26 - 10, Color(2, 9, 46, 200), 35);
        drawString(ImVec2(Screen2.x + 1, Screen2.y - 1), Color(255, 255, 255, 255), true, false, skeet_menu.weapon_font, "j");
    }
}

void grenade_warning::render_inc_projectile() {
    Vector Screen;
    if (math::world_to_screen(end, Screen))
    {
        imRender->CircleFilled(Screen.x, Screen.y, 26 - 10 + 5, safe ? Color(170, 170, 255, 190) : Color(255, 44, 20, 255), 35);
        imRender->CircleFilled(Screen.x, Screen.y, 26 - 10, Color(2, 9, 46, 200), 35);
        drawString(ImVec2(Screen.x + 1, Screen.y - 1), Color(255, 255, 255, 255), true, false, skeet_menu.weapon_font, "l");
    }
    else if (distance <= 735) {

        Vector dir;
        m_engine()->GetViewAngles(dir);

        float ViewAngle = dir.y;

        if (ViewAngle < 0.f)
        {
            ViewAngle += 360.f;
        }
        ViewAngle = DEG2RAD(ViewAngle);

        auto TowardsAngle = math::calculate_angle(cheat.local()->m_vecOrigin(), end);
        TowardsAngle.Normalized();

        if (TowardsAngle.y < 0.f) {
            TowardsAngle.y += 360.f;
        }
        TowardsAngle.y = DEG2RAD(TowardsAngle.y);
        TowardsAngle.y -= ViewAngle;
        Vector2D Screen2;
        Screen2 = Vector2D(1920 / 2 - 340, 1080 / 2 - 200);
        rotate_point(Screen2, Vector2D(1920 / 2, 1080 / 2), false, TowardsAngle.y);

        imRender->CircleFilled(Screen2.x, Screen2.y, 26 - 10 + 5, safe ? Color(170, 170, 255, 190) : Color(255, 44, 20, 255), 35);
        imRender->CircleFilled(Screen2.x, Screen2.y, 26 - 10, Color(2, 9, 46, 200), 35);
        drawString(ImVec2(Screen2.x + 1, Screen2.y - 1), Color(255, 255, 255, 255), true, false, skeet_menu.weapon_font, "l");
    }
}
