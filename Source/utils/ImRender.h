#pragma once
#include "..\includes.hpp"
#include "..\utils\UnicodeFonts.h"

enum IFontCenteringFlags {
	IFONT_NONE = 1 << 0,
	IFONT_CENTERED_X = 1 << 1,
	IFONT_CENTERED_Y = 1 << 2,
	IFONT_OUTLINE = 1 << 3
};

namespace render2 {
	enum e_cornerflags {
		corner_left_top = 1 << 0,
		corner_right_top = 1 << 1,
		corner_left_bot = 1 << 2,
		corner_right_bot = 1 << 3,
		corner_top = ImDrawCornerFlags_TopLeft | ImDrawCornerFlags_TopRight,   // 0x3
		corner_bot = ImDrawCornerFlags_BotLeft | ImDrawCornerFlags_BotRight,   // 0xC
		corner_left = ImDrawCornerFlags_TopLeft | ImDrawCornerFlags_BotLeft,    // 0x5
		corner_right = ImDrawCornerFlags_TopRight | ImDrawCornerFlags_BotRight,  // 0xA
	};
}

class ImGuiRendering {
public:
	void filled_rect_world(Vector center, Vector2D size, Color color, int angle, float damage);
	void BeginScene();
	void EndScene();
	void DrawSphere(Vector origin, float radius, float angle, float scale, Color color);
	void arc(float x, float y, float radius, float min_angle, float max_angle, Color col, float thickness);
	void two_sided_arc(float x, float y, float radius, float scale, Color col, float thickness);
	void DrawLine(float x1, float y1, float x2, float y2, Color clr, float thickness = 1.0F);
	void PolyLine(const ImVec2* points, const int points_count, Color clr, bool closed, float thickness = 1.f, Color fill = Color(0, 0, 0, 0));
	void DrawLineGradient(float x1, float y1, float x2, float y2, Color clr1, Color cl2, float thickness = 1.f, bool flip = false);
	void Rect(float x, float y, float w, float h, Color clr, float rounding = 0.f, ImDrawCornerFlags flags = 15);
	void FilledRect(float x, float y, float w, float h, Color clr, float rounding = 0.f, ImDrawCornerFlags flags = 15);
	void FilledRect2(float x, float y, float w, float h, Color clr, float rounding = 0.f, ImDrawCornerFlags flags = 15);
	void Triangle(float x1, float y1, float x2, float y2, float x3, float y3, Color clr, float thickness = 1.f);
	void TriangleFilled(float x1, float y1, float x2, float y2, float x3, float y3, Color clr);
	void QuadFilled(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, Color clr);

	void Circle(float x1, float y1, float radius, Color col, int segments);

	void CircleFilled(float x1, float y1, float radius, Color col, int segments);
	void DrawWave(Vector loc, float radius, Color clr, float thickness = 1.f);
	void DrawString(float x, float y, Color color, int flags, ImFont* font, const char* message, ...);
	void DrawString(float x, float y, Color color, int flags, ImFont* font, const std::string message, ...);
	void DrawStringWithShadow(float x, float y, Color color, int flags, ImFont* font, const char* message, ...);
	void DrawStringWithShadow(float x, float y, Color color, int flags, ImFont* font, const std::string message, ...);
	void DrawCircle(float x, float y, float r, float s, Color color);
	void DrawGradientCircle(float x, float y, float rad, float rotate, int type, int resolution, DWORD color, DWORD color2);
	ImVec2 GetTextSize(ImFont* font, std::string text);
	void DrawRing3D(int16_t x, int16_t y, int16_t z, int16_t radius, uint16_t points, Color color1, Color color2, float thickness, float progress = 1.f, bool fill_prog = false);
	void DrawCollisionRing3D(int16_t x, int16_t y, int16_t z, int16_t radius, uint16_t points, Color color1, Color color2, float thickness, float progress = 1.f);
	void DrawDepthRing3D(int16_t x, int16_t y, int16_t z, int16_t radius, uint16_t points, Color color1, Color color2, float thickness, float progress = 1.f);
};

inline ImGuiRendering* imRender;