#include "UnicodeFonts.h"

#include "../utils/FontUtils.h"
#include "../features/menu.h"

#include <ShlObj_core.h>

static ImFont* addFontFromVFONT(const std::string& path, float size, const ImWchar* glyphRanges, bool merge) noexcept
{
	auto file = FontUtils::loadBinaryFile(path);
	if (!FontUtils::decodeVFONT(file))
		return nullptr;

	ImFontConfig cfg;
	cfg.FontData = file.data();
	cfg.FontDataSize = file.size();
	cfg.FontDataOwnedByAtlas = false;
	cfg.MergeMode = merge;
	cfg.GlyphRanges = glyphRanges;
	cfg.SizePixels = size;

	return ImGui::GetIO().Fonts->AddFont(&cfg);
}

void UnicodeFonts::init(ImGuiIO io)
{
	ImFontConfig cfg;
	cfg.SizePixels = 15.0f;
	if (PWSTR pathToFonts; SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Fonts, 0, nullptr, &pathToFonts))) {
		const std::filesystem::path path{ pathToFonts };
		CoTaskMemFree(pathToFonts);

		fonts.normal15px = io.Fonts->AddFontFromFileTTF((path / "tahoma.ttf").string().c_str(), 15.0f, &cfg, FontUtils::getFontGlyphRanges());
		if (!fonts.normal15px)
			io.Fonts->AddFontDefault(&cfg);

		cfg.MergeMode = true;
		static constexpr ImWchar symbol[]{
			0x2605, 0x2605, // бя
			0
		};
		io.Fonts->AddFontFromFileTTF((path / "seguisym.ttf").string().c_str(), 15.0f, &cfg, symbol);
		cfg.MergeMode = false;
	}
	if (!fonts.normal15px)
		io.Fonts->AddFontDefault(&cfg);
	addFontFromVFONT("csgo/panorama/fonts/notosanskr-regular.vfont", 15.0f, io.Fonts->GetGlyphRangesKorean(), true);
	fonts.chinese = addFontFromVFONT("csgo/panorama/fonts/notosanssc-regular.vfont", 17.0f, io.Fonts->GetGlyphRangesChineseFull(), true);
	fonts.chinese14 = addFontFromVFONT("csgo/panorama/fonts/notosanssc-regular.vfont", 14.0f, io.Fonts->GetGlyphRangesChineseFull(), true);
	fonts.chinese10 = addFontFromVFONT("csgo/panorama/fonts/notosanssc-regular.vfont", 10.0f, io.Fonts->GetGlyphRangesChineseFull(), true);
}