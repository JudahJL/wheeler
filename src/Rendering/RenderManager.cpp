#include <SimpleIni.h>
#include <d3d11.h>

#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

// #include <dxgi.h>

// #include <imgui_internal.h>
// stole this from MaxSu's detection meter
#include <imgui_freetype.h>

#include "Rendering/RenderManager.h"
#include "Wheeler/Wheeler.h"
#include "Rendering/TextureManager.h"

#include "Animation/TimeInterpolator/TimeInterpolatorManager.h"

namespace stl {
    using namespace SKSE::stl;

    template<class T>
    void write_thunk_call() {
        auto&                                 trampoline{ SKSE::GetTrampoline() };
        const REL::Relocation<std::uintptr_t> hook{ T::id, T::offset };
        T::func = trampoline.write_call<5>(hook.address(), T::thunk);
    }
}  // namespace stl

LRESULT RenderManager::WndProcHook::thunk(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    auto& io{ ImGui::GetIO() };
    if(uMsg == WM_KILLFOCUS) {
        io.ClearInputCharacters();
        io.ClearInputKeys();
    }

    return func(hWnd, uMsg, wParam, lParam);
}

void RenderManager::D3DInitHook::thunk() {
    func();

    logger::info("RenderManager: Initializing...");
    const auto* render_manager{ RE::BSGraphics::Renderer::GetSingleton() };
    if(!render_manager) [[unlikely]] {
        logger::error("Cannot find render manager. Initialization failed!");
        return;
    }

    const auto& render_data{ render_manager->GetRuntimeData() };

    logger::info("Getting swapchain...");
    const auto swapchain{ render_data.renderWindows[0].swapChain };
    if(!swapchain) [[unlikely]] {
        logger::error("Cannot find swapchain. Initialization failed!");
        return;
    }

    logger::info("Getting swapchain desc...");
    REX::W32::DXGI_SWAP_CHAIN_DESC sd{};
    if(swapchain->GetDesc(std::addressof(sd)) < 0) [[unlikely]] {
        logger::error("IDXGISwapChain::GetDesc failed.");
        return;
    }

    device           = render_data.forwarder;
    Texture::device_ = device;
    context          = render_data.context;

    logger::info("Initializing ImGui...");
    ImGui::CreateContext();
    if(!ImGui_ImplWin32_Init(sd.outputWindow)) [[unlikely]] {
        logger::error("ImGui initialization failed (Win32)");
        return;
    }
    if(!ImGui_ImplDX11_Init(reinterpret_cast<ID3D11Device*>(device), reinterpret_cast<ID3D11DeviceContext*>(context))) [[unlikely]] {
        logger::error("ImGui initialization failed (DX11)");
        return;
    }

    logger::info("...ImGui Initialized");

    initialized.store(true);

    WndProcHook::func = reinterpret_cast<WNDPROC>(
        SetWindowLongPtrA(
            sd.outputWindow,
            GWLP_WNDPROC,
            reinterpret_cast<LONG_PTR>(WndProcHook::thunk)));

    if(!WndProcHook::func) {
        logger::error("SetWindowLongPtrA failed!");
    }

    logger::info("Building font atlas...");
    std::filesystem::path fontPath;
    bool                  foundCustomFont{ false };
    const ImWchar*        glyphRanges{ nullptr };
    constexpr auto        FONTSETTING_PATH{ R"(Data\SKSE\Plugins\wheeler\resources\fonts\FontConfig.ini)" };
    CSimpleIniA           ini;
    ini.SetUnicode();
    if(ini.LoadFile(FONTSETTING_PATH) < 0) {
        logger::error("Failed to load fontConfig.ini");
    }
    if(!ini.IsEmpty()) {
        const std::string language{ ini.GetValue("config", "font", nullptr) };
        logger::trace("Language: {}", language);
        if(!language.empty()) [[unlikely]] {
            const std::string fontDir{ std::format(R"(Data\SKSE\Plugins\wheeler\resources\fonts\{})", language) };
            // check if folder exists
            if(std::filesystem::exists(fontDir) && std::filesystem::is_directory(fontDir)) {
                for(const auto& entry : std::filesystem::directory_iterator(fontDir)) {
                    const auto& entryPath{ entry.path() };
                    if(entryPath.extension() == ".ttf" || entryPath.extension() == ".ttc") {
                        logger::trace("Font: {}", entryPath.string());
                        fontPath        = entryPath;
                        foundCustomFont = true;
                        break;
                    }
                }
            } else {
                logger::error("{} doesnt exist or isn't a directory", fontDir);
            }
            if(foundCustomFont) {
                logger::info("Loading font type: {}", fontPath.string().c_str());
                if(language == "Chinese") {
                    logger::info("Glyph range set to Chinese");
                    glyphRanges = ImGui::GetIO().Fonts->GetGlyphRangesChineseFull();
                } else if(language == "Korean") {
                    logger::info("Glyph range set to Korean");
                    glyphRanges = ImGui::GetIO().Fonts->GetGlyphRangesKorean();
                } else if(language == "Japanese") {
                    logger::info("Glyph range set to Japanese");
                    glyphRanges = ImGui::GetIO().Fonts->GetGlyphRangesJapanese();
                } else if(language == "Thai") {
                    logger::info("Glyph range set to Thai");
                    glyphRanges = ImGui::GetIO().Fonts->GetGlyphRangesThai();
                } else if(language == "Vietnamese") {
                    logger::info("Glyph range set to Vietnamese");
                    glyphRanges = ImGui::GetIO().Fonts->GetGlyphRangesVietnamese();
                } else if(language == "Cyrillic") {
                    glyphRanges = ImGui::GetIO().Fonts->GetGlyphRangesCyrillic();
                    logger::info("Glyph range set to Cyrillic");
                }
            } else {
                logger::info("No font glyph set for language: {}", language);
            }
        }
    }
#define ENABLE_FREETYPE 1
#if ENABLE_FREETYPE
    {
        ImFontAtlas* atlas{ ImGui::GetIO().Fonts };
        atlas->FontBuilderIO    = ImGuiFreeType::GetBuilderForFreeType();
        atlas->FontBuilderFlags = ImGuiFreeTypeBuilderFlags_LightHinting;
    }
#else
#endif
    if(foundCustomFont) {
        const auto font_path_str{fontPath.string()};
        ImGui::GetIO().Fonts->AddFontFromFileTTF(font_path_str.data(), 64.0f, nullptr, glyphRanges);
        logger::trace("Found Font: {}", font_path_str);
    } else {
        ImGui::GetIO().Fonts->AddFontDefault();
    }

    logger::info("...font atlas built");

    logger::info("RenderManager: Initialized");
}

void RenderManager::DXGIPresentHook::thunk(std::uint32_t a_p1) {
    func(a_p1);

    if(!D3DInitHook::initialized.load()) {
        return;
    }

    // prologue
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // do stuff
    RenderManager::draw();

    // epilogue
    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

struct ImageSet {
    std::int32_t                        my_image_width{ 0 };
    std::int32_t                        my_image_height{ 0 };
    REX::W32::ID3D11ShaderResourceView* my_texture{ nullptr };
};

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void RenderManager::MessageCallback(SKSE::MessagingInterface::Message* msg)  //CallBack & LoadTextureFromFile should called after resource loaded.
{
    if(msg->type == SKSE::MessagingInterface::kDataLoaded && D3DInitHook::initialized) {
        auto& io{ ImGui::GetIO() };
        io.MouseDrawCursor = true;
        io.WantSetMousePos = true;
    }
}

bool RenderManager::Install() {
    const auto g_message{ SKSE::GetMessagingInterface() };
    if(!g_message) {
        util::report_and_fail("Messaging Interface Not Found!");
    }

    g_message->RegisterListener(MessageCallback);

    SKSE::AllocTrampoline(14 * 2);

    stl::write_thunk_call<D3DInitHook>();
    stl::write_thunk_call<DXGIPresentHook>();

    return true;
}

float RenderManager::GetResolutionScaleWidth() {
    return ImGui::GetIO().DisplaySize.x / 1920.f;
}

float RenderManager::GetResolutionScaleHeight() {
    return ImGui::GetIO().DisplaySize.y / 1080.f;
}

void RenderManager::draw() {
    // Add UI elements here
    float deltaTime{ ImGui::GetIO().DeltaTime };
    Wheeler::Update(deltaTime);
    TimeFloatInterpolatorManager::Update(deltaTime);
}
