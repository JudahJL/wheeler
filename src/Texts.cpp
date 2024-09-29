#include "Texts.h"

static void loadTextFromIni(CSimpleIniA& a_ini, std::string& r_text)
{
	r_text = std::string(a_ini.GetValue("Texts", r_text.data(), r_text.data()));
}
void Texts::LoadTranslations()
{
	CSimpleIniA ini;
	ini.SetUnicode();
    constexpr auto TEXTS_PATH{ R"(Data\SKSE\Plugins\wheeler\Texts.ini)" };
	ini.LoadFile(TEXTS_PATH);
	try {
		for (auto& text : _textData | std::views::values) {
			loadTextFromIni(ini, text);
		}
	}
	catch (const std::exception& e) {
		logger::error("Error loading from Texts.ini: {}", e.what());
	}

}

const char* Texts::GetText(const TextType a_textType)
{
	return _textData[a_textType].data();
}
