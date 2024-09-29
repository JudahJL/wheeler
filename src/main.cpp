// #include "UserInput/Input.h"
#include "UserInput/Controls.h"

#include "Rendering/RenderManager.h"
#include "Rendering/TextureManager.h"

#include "Hooks.h"

#include "Wheeler/WheelItems/WheelItemMutableManager.h"
#include "Wheeler/Wheeler.h"
#include "Utilities/UniqueIDHandler.h"
#include "Serialization/SerializationEntry.h"

#include "Config.h"
#include "Texts.h"
#include "ModCallbackEventHandler.h"

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void MessageHandler(SKSE::MessagingInterface::Message* a_msg) {
    switch(a_msg->type) {
        case SKSE::MessagingInterface::kDataLoaded:
            WheelItemMutableManager::GetSingleton()->Register();
            Config::ReadStyleConfig();
            Config::ReadControlConfig();
            Config::OffsetSizingToViewport();
            Controls::BindAllInputsFromConfig();
            Texture::Init();
            Texts::LoadTranslations();
            ModCallbackEventHandler::Register();
            break;
        case SKSE::MessagingInterface::kNewGame:
            Wheeler::SetupDefaultWheels();
            break;
        case SKSE::MessagingInterface::kPostLoadGame:
            UniqueIDHandler::EnsureXListUniquenessInPcInventory();
            break;
        default:
            break;
    }
}

void onSKSEInit() {
    RenderManager::Install();
    Wheeler::Init();
    Hooks::Install();
    const auto serialization{ SKSE::GetSerializationInterface() };
    serialization->SetUniqueID(WHEELER_SERIALIZATION_ID);
    SerializationEntry::BindSerializationCallbacks(serialization);
}

namespace {
    void InitializeLog(const SKSE::PluginDeclaration* plugin = SKSE::PluginDeclaration::GetSingleton()) {
        auto  log{ std::make_shared<spdlog::logger>(plugin->GetName().data()) };
        auto& sink{ log->sinks() };
        if(IsDebuggerPresent()) [[unlikely]] {
            sink.reserve(2);
            sink.emplace_back(std::make_shared<spdlog::sinks::msvc_sink_mt>());
        }
        auto path{ logger::log_directory() };
        if(!path) {
            util::report_and_fail("Failed to find standard logging directory");
        }
        *path /= std::format("{}.log", plugin->GetName());
        sink.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true));

        constexpr auto level{ spdlog::level::trace };
        log->set_level(level);
        log->flush_on(level);
        log->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] [%t] [%s:%#] %v");
        spdlog::set_default_logger(std::move(log));
    }
}  // namespace

std::string wstring2string(const std::wstring& wstr, const UINT CodePage)

{
    std::string ret;

    const int len = WideCharToMultiByte(CodePage, 0, wstr.c_str(), static_cast<int>(wstr.size()), nullptr, 0, nullptr, nullptr);

    ret.resize(static_cast<size_t>(len), 0);

    WideCharToMultiByte(CodePage, 0, wstr.c_str(), static_cast<int>(wstr.size()), &ret[0], len, nullptr, nullptr);

    return ret;
}

SKSEPluginLoad(const SKSE::LoadInterface* a_skse) {
// #define SKYRIM_WHEELER_ATTACH_DEBUGGER_FOR_NOW
#ifdef SKYRIM_WHEELER_ATTACH_DEBUGGER_FOR_NOW
    while(!REX::W32::IsDebuggerPresent()) {
        std::this_thread::sleep_for(std::chrono::seconds{ 5 });
    }
#endif
    const SKSE::PluginDeclaration* plugin{ SKSE::PluginDeclaration::GetSingleton() };
    InitializeLog();
    logger::info("{} v{}", plugin->GetName(), plugin->GetVersion().string("."));

    SKSE::Init(a_skse);

    auto messaging = SKSE::GetMessagingInterface();
    if(!messaging->RegisterListener("SKSE", MessageHandler)) {
        return false;
    }

    onSKSEInit();

    return true;
}
