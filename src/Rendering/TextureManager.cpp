#include "Rendering/TextureManager.h"
#define NANOSVG_IMPLEMENTATION
#define NANOSVG_ALL_COLOR_KEYWORDS
#include <nanosvg.h>
#define NANOSVGRAST_IMPLEMENTATION
#include <nanosvgrast.h>

void Texture::Init() {
    //load_images(image_type_name_map, image_struct, img_directory);
    load_images(icon_type_name_map, icon_struct, icon_directory);
    load_custom_icon_images();
    //load_images(key_icon_name_map, key_struct, key_directory);
    //load_images(default_key_icon_name_map, default_key_struct, key_directory);
    //load_images(gamepad_ps_icon_name_map, ps_key_struct, key_directory);
    //load_images(gamepad_xbox_icon_name_map, xbox_key_struct, key_directory);
}

Texture::Image Texture::GetIconImage(icon_image_type a_imageType, RE::TESForm* a_form) {
    // look for formId matches
    if(a_form) {
        if(icon_struct_formID.contains(a_form->GetFormID())) {
            return icon_struct_formID[a_form->GetFormID()];
        }
        // look for keyword matches
        if(const auto keywordForm{ a_form->As<RE::BGSKeywordForm>() }; keywordForm) {
            for(auto& [fst, snd] : icon_struct_keyword) {
                if(keywordForm->HasKeywordString(fst)) {
                    return snd;
                }
            }
        }
    }
    // look for type matches
    return icon_struct[static_cast<int32_t>(a_imageType)];
}

bool Texture::load_texture_from_file(const char* filename, Image& a_image) {
    if(!device_) {
        util::report_and_fail("device can't be null");
    }

    auto* render_manager{ RE::BSGraphics::Renderer::GetSingleton() };
    if(!render_manager) [[unlikely]] {
        logger::error("Cannot find render manager. Initialization failed.");
        return false;
    }

    const auto& render_data{ render_manager->GetRuntimeData() };
    {
        auto* svg{ nsvgParseFromFile(filename, "px", 96.0f) };
        auto* rast{ nsvgCreateRasterizer() };

        const auto image_width{ static_cast<uint32_t>(svg->width) };
        const auto image_height{ static_cast<uint32_t>(svg->height) };

        const auto image_data{ static_cast<unsigned char*>(malloc(image_width * image_height * 4)) };
        nsvgRasterize(rast, svg, 0, 0, 1, image_data, image_width, image_height, image_width * 4);
        nsvgDelete(svg);
        nsvgDeleteRasterizer(rast);
        {
            // Create texture
            const REX::W32::D3D11_TEXTURE2D_DESC desc{
                image_width,
                image_height,
                1, 1,
                REX::W32::DXGI_FORMAT_R8G8B8A8_UNORM,
                REX::W32::DXGI_SAMPLE_DESC{ 1 },
                REX::W32::D3D11_USAGE_DEFAULT,
                REX::W32::D3D11_BIND_SHADER_RESOURCE,
                0, 0
            };

            REX::W32::ID3D11Texture2D*             p_texture{ nullptr };
            const REX::W32::D3D11_SUBRESOURCE_DATA sub_resource{
                image_data,
                desc.width * 4,
                0
            };

            auto result_of_creste_texture{ device_->CreateTexture2D(&desc, &sub_resource, &p_texture) };
            if(FAILED(result_of_creste_texture)) {
                char* errorMsg{ nullptr };
                FormatMessageA(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    nullptr, result_of_creste_texture, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    reinterpret_cast<LPSTR>(&errorMsg), 0, nullptr);
                if(errorMsg) {
                    logger::error("CreateTexture2D, file {}, HRESULT: 0x{:X}, MSG: {}",
                                  filename, result_of_creste_texture, errorMsg);
                    LocalFree(errorMsg);  // Release the memory allocated for the error message
                } else {
                    logger::error("CreateTexture2D, file {}, HRESULT: 0x{:X}, MSG: Unable to retrieve error message",
                                  filename, result_of_creste_texture);
                }
            }
            {
                // Create texture view
                const REX::W32::D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{
                    .format{ REX::W32::DXGI_FORMAT_R8G8B8A8_UNORM },
                    .viewDimension{ REX::W32::D3D11_SRV_DIMENSION_TEXTURE2D },
                    .texture2D{ 0, desc.mipLevels }
                };
                {
                    auto result_of_creste_shader_resource{ render_data.forwarder->CreateShaderResourceView(p_texture, &srv_desc, &a_image.texture) };
                    if(FAILED(result_of_creste_shader_resource)) {
                        char* errorMsg{ nullptr };
                        FormatMessageA(
                            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                            nullptr, result_of_creste_shader_resource, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                            reinterpret_cast<LPSTR>(&errorMsg), 0, nullptr);
                        if(errorMsg) {
                            logger::trace("CreateShaderResourceView, file {}, HRESULT: 0x{:X}, Error MSG: {}",
                                          filename, result_of_creste_shader_resource, errorMsg);
                            LocalFree(errorMsg);  // Release the memory allocated for the error message
                        } else {
                            logger::trace("CreateShaderResourceView, file {}, HRESULT: 0x{:X}, Error MSG: Unable to retrieve error message",
                                          filename, result_of_creste_shader_resource);
                        }
                    }
                    p_texture->Release();

                    free(image_data);

                    a_image.width  = image_width;
                    a_image.height = image_height;

                    return true;
                }
            }
        }
    }
}

static RE::FormID hexStringToInt(const std::string& hexString) {
    try {
        // Using std::stoul to convert hex string, base 16 will handle "0x" and whitespaces
        const auto form_id{ static_cast<RE::FormID>(std::stoul(hexString, nullptr, 16)) };
        return form_id;
    } catch(const std::invalid_argument& e) {
        // Handle invalid hex string
        logger::error("{}", e.what());
    } catch(const std::out_of_range& e) {
        // Handle out-of-range values
        logger::error("{}", e.what());
    }
    return {};
}

void Texture::load_custom_icon_images() {
    const auto tesDataHandler{ RE::TESDataHandler::GetSingleton() };
    if(!tesDataHandler) [[unlikely]] {
        return;
    }
    for(const auto& entry : std::filesystem::directory_iterator(icon_custom_directory)) {
        if(entry.path().filename().extension() != ".svg") {
            continue;
        }
        std::string fileName{ entry.path().filename().string() };
        bool        fid{ false };  // whether we're looking for a formID match
        if(size_t idx{ fileName.find("FID_") }; idx == 0) {
            fid = true;
        } else {
            idx = fileName.find("KWD_");
            if(idx != 0) {
                continue;  // not a valid file name
            }
        }
        if(fid) {
            constexpr size_t pluginNameBegin{ 4 };
            size_t           pluginNameEnd{ fileName.find("_0x", pluginNameBegin) };  // find the 2nd '_'
            if(pluginNameEnd == std::string::npos) {
                pluginNameEnd = fileName.find("_0X", pluginNameBegin);
                if(pluginNameEnd == std::string::npos) {
                    continue;
                }
            }
            {
                std::string        pluginName{ fileName.substr(pluginNameBegin, pluginNameEnd - pluginNameBegin) };
                const size_t       formIdEnd{ fileName.find(".svg") };
                const size_t       formIdBegin{ pluginNameEnd + 1 };
                std::string        formIdStr{ fileName.substr(formIdBegin, formIdEnd - formIdBegin) };
                const auto         formId{ hexStringToInt(formIdStr) };
                const RE::TESForm* form{ tesDataHandler->LookupForm(formId, pluginName) };
                if(!form) {
                    continue;
                }
                RE::FormID formID{ form->GetFormID() };
                Image      img;
                load_texture_from_file(entry.path().string().c_str(), img);
                icon_struct_formID[formID] = img;
            }
        } else {  //kwd
            constexpr size_t keywordBegin{ 4 };
            const size_t     keywordEnd{ fileName.find(".svg") };
            std::string      keyWord{ fileName.substr(keywordBegin, keywordEnd - keywordBegin) };
            Image            img;
            load_texture_from_file(entry.path().string().c_str(), img);
            icon_struct_keyword[keyWord] = img;
        }
    }
}
