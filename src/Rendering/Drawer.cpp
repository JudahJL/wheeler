#include <imgui.h>
#include <imgui_internal.h>

#include "Rendering/Drawer.h"
#include "Utilities/Utils.h"
#include "Config.h"

void Drawer::draw_text(const float     a_x,
                       const float     a_y,
                       const char*     a_text,
                       ImU32           a_color,
                       const float     a_font_size,
                       const DrawArgs& a_drawArgs,
                       const bool      a_center_text) {
    const auto* font{ ImGui::GetDefaultFont() };  // TODO: add custom font support

    if(!a_text || !*a_text) {
        return;
    }

    if(a_drawArgs.alphaMultiplier == 0.f || a_font_size == 0) {
        return;
    }

    Utils::Color::MultAlpha(a_color, a_drawArgs.alphaMultiplier);

    ImVec2 position{ a_x, a_y };

    if(a_center_text) {
        const float currFontSize{ ImGui::GetFontSize() };

        const float fontRatio{ a_font_size / currFontSize };

        ImVec2 text_size{ ImGui::CalcTextSize(a_text) };
        text_size.x *= fontRatio;
        text_size.y *= fontRatio;

        //it should center the text, it kind of does
        auto text_x{ 0.f };
        auto text_y{ 0.f };

        text_x = -text_size.x * 0.5f;
        text_y = -text_size.y * 0.5f;

        position.x += text_x;
        position.y += text_y;
    }

    const auto drawList{ ImGui::GetWindowDrawList() };

    ImVec2      shadowPos(position);
    const float shadowOffset{ a_font_size * 0.05f };

    shadowPos.x += shadowOffset;
    shadowPos.y += shadowOffset;

    ImU32 shadowCol{ Config::Styling::Wheel::TextShadowColor };
    Utils::Color::MultAlpha(shadowCol, a_drawArgs.alphaMultiplier);
    // draw shadow
    drawList->AddText(font, a_font_size, shadowPos, shadowCol, a_text, nullptr, 0.0f, nullptr);
    // lay text on top of shadow
    drawList->AddText(font, a_font_size, position, a_color, a_text, nullptr, 0.0f, nullptr);
}

/// <summary>
/// Splits a text into multiple lines and storing them in a vector,
/// given a maximum width.
/// </summary>
/// <param name="a_text"></param>
/// <param name="a_max_width"></param>
/// <param name="a_font_size"></param>
/// <param name="r_text_lines"></param>
static void split_text_lines_utf8(const std::string_view a_text, const float a_max_width, const float a_font_size, std::vector<std::string>& r_text_lines) {
    const float  raw_length{ ImGui::CalcTextSize(a_text.data()).x * (a_font_size / ImGui::GetFontSize()) };
    const size_t num_lines(std::ceil(raw_length / a_max_width));
    const size_t line_length(std::ceil(a_text.length() / num_lines));
    size_t       curr{};
    for(size_t i{}; i < num_lines; i++) {
        if(i == num_lines - 1) {
            if(curr >= a_text.length()) {
                break;
            }
            r_text_lines.emplace_back(a_text.substr(curr));
        } else {
            size_t end{ curr + line_length };
            // adjust to the end of the last UTF-8 sequence
            while(end < a_text.length() && (a_text[end] & 0xC0) == 0x80) {
                --end;
            }
            size_t last_space = a_text.rfind(' ', end);
            // adjust to the end of the last UTF-8 sequence
            while(last_space > curr && (a_text[last_space] & 0xC0) == 0x80) {
                --last_space;
            }
            if(last_space != std::string::npos && last_space > curr) {
                r_text_lines.emplace_back(a_text.substr(curr, last_space - curr));
                curr = last_space + 1;
            } else {
                r_text_lines.emplace_back(a_text.substr(curr, end - curr));
                curr = end;
            }
        }
    }
}

/// <summary>
/// Helper to split a text into multiple line, storing them in R_TEXT_LINES.
/// Calculates the length of the whole text and divide it into mulple lines of approximately same length.
/// Words(consequtive characters) won't be split mid-line.
/// </summary>
// static void split_text_lines(const std::string& a_text, float a_max_width, float a_font_size, std::vector<std::string>& r_text_lines) {
//     const float  raw_length{ ImGui::CalcTextSize(a_text.c_str()).x * (a_font_size / ImGui::GetFontSize()) };  // raw length of text in pixels
//     const float  num_lines{ ceil(raw_length / a_max_width) };
//     const float  line_length{ ceil(a_text.length() / num_lines) };
//     size_t curr{};
//     for(size_t i{}; i < num_lines; i++) {
//         if(i == num_lines - 1) {  // last line, push everything
//             if(curr >= a_text.length()) {
//                 break;
//             }
//             r_text_lines.push_back(a_text.substr(curr));
//         } else {
//             const auto last_space{ a_text.rfind(' ', curr + line_length) };
//             if(last_space != std::string::npos) {
//                 r_text_lines.push_back(a_text.substr(curr, last_space - curr));
//                 curr = last_space + 1;
//             } else {
//                 r_text_lines.push_back(a_text.substr(curr, line_length));
//                 curr += line_length;
//             }
//         }
//     }
// }

void Drawer::draw_text_block(const float a_x, float a_y, const std::string_view a_text, const ImU32 a_color, const float a_font_size, const float a_line_spacing, const float a_line_length, const DrawArgs& a_drawArgs) {
    if(a_text.empty()) {
        return;
    }
    if(a_drawArgs.alphaMultiplier == 0.f || a_font_size == 0) {
        return;
    }
    std::vector<std::string> text_lines;
    split_text_lines_utf8(a_text, a_line_length, a_font_size, text_lines);

    const float font_height{ ImGui::CalcTextSize("t").y * a_font_size / ImGui::GetFontSize() };
    for(const auto& line : text_lines) {
        draw_text(a_x, a_y, line.c_str(), a_color, a_font_size, a_drawArgs);
        a_y += (font_height + a_line_spacing);
    }
}

void Drawer::draw_texture(REX::W32::ID3D11ShaderResourceView* a_texture,
                          ImVec2                              a_center,
                          const float                         a_offset_x,
                          const float                         a_offset_y,
                          const ImVec2                        a_size,
                          ImU32                               a_color,
                          const DrawArgs&                     a_drawArgs) {
    a_center = ImVec2(a_center.x + a_offset_x, a_center.y + a_offset_y);
    const float      cos_a{ cosf(a_drawArgs.rotationOffset) };
    const float      sin_a{ sinf(a_drawArgs.rotationOffset) };
    const std::array temp{
        ImRotate(ImVec2{ -a_size.x * 0.5f, -a_size.y * 0.5f }, cos_a, sin_a),
        ImRotate(ImVec2{ +a_size.x * 0.5f, -a_size.y * 0.5f }, cos_a, sin_a),
        ImRotate(ImVec2{ +a_size.x * 0.5f, +a_size.y * 0.5f }, cos_a, sin_a),
        ImRotate(ImVec2{ -a_size.x * 0.5f, +a_size.y * 0.5f }, cos_a, sin_a)
    };
    const std::array<ImVec2, 4> pos{
        { { a_center.x + temp.at(0).x, a_center.y + temp.at(0).y },
         { a_center.x + temp.at(1).x, a_center.y + temp.at(1).y },
         { a_center.x + temp.at(2).x, a_center.y + temp.at(2).y },
         { a_center.x + temp.at(3).x, a_center.y + temp.at(3).y } }
    };
    constexpr std::array<ImVec2, 4> uvs{
        { { 0.0f, 0.0f },
         { 1.0f, 0.0f },
         { 1.0f, 1.0f },
         { 0.0f, 1.0f } }
    };

    Utils::Color::MultAlpha(a_color, a_drawArgs.alphaMultiplier);
    ImGui::GetWindowDrawList()
        ->AddImageQuad(a_texture, pos[0], pos[1], pos[2], pos[3], uvs[0], uvs[1], uvs[2], uvs[3], a_color);
}

void Drawer::draw_arc(ImVec2 center,
                      float radius_min, float radius_max,
                      float inner_ang_min, float inner_ang_max,
                      float outer_ang_min, float outer_ang_max,
                      ImU32 a_color, uint32_t segments, const DrawArgs& a_drawArgs) {
    auto drawList{ ImGui::GetWindowDrawList() };

    const float fAngleStepInner{ (inner_ang_max - inner_ang_min) / segments };
    const float fAngleStepOuter{ (outer_ang_max - outer_ang_min) / segments };

    const ImVec2& vTexUvWhitePixel = ImGui::GetDrawListSharedData()->TexUvWhitePixel;
    // draw an arc for the current item
    Utils::Color::MultAlpha(a_color, a_drawArgs.alphaMultiplier);

    drawList->PrimReserve(segments * 6, (segments + 1) * 2);
    for(int iSeg{}; iSeg <= segments; ++iSeg) {
        const float fCosInner{ cosf(inner_ang_min + fAngleStepInner * iSeg) };
        const float fSinInner{ sinf(inner_ang_min + fAngleStepInner * iSeg) };
        const float fCosOuter{ cosf(outer_ang_min + fAngleStepOuter * iSeg) };
        const float fSinOuter{ sinf(outer_ang_min + fAngleStepOuter * iSeg) };

        if(iSeg < segments) {
            drawList->PrimWriteIdx(drawList->_VtxCurrentIdx + 0);
            drawList->PrimWriteIdx(drawList->_VtxCurrentIdx + 2);
            drawList->PrimWriteIdx(drawList->_VtxCurrentIdx + 1);
            drawList->PrimWriteIdx(drawList->_VtxCurrentIdx + 3);
            drawList->PrimWriteIdx(drawList->_VtxCurrentIdx + 2);
            drawList->PrimWriteIdx(drawList->_VtxCurrentIdx + 1);
        }

        drawList->PrimWriteVtx(ImVec2{ center.x + fCosInner * radius_min, center.y + fSinInner * radius_min }, vTexUvWhitePixel, a_color);
        drawList->PrimWriteVtx(ImVec2{ center.x + fCosOuter * radius_max, center.y + fSinOuter * radius_max }, vTexUvWhitePixel, a_color);
    }
}

inline uint32_t bigLerp(const uint32_t a, const uint32_t b, const double t) {
    return a + (b - a) * t;
}

void Drawer::draw_arc_gradient(const ImVec2 center, const float radius_min, const float radius_max,
                               const float inner_ang_min, const float inner_ang_max,
                               const float outer_ang_min, const float outer_ang_max,
                               const ImU32 color_start, const ImU32 color_end, const uint32_t segments, const DrawArgs& a_drawArgs) {
    const float fAngleStepInner{ (inner_ang_max - inner_ang_min) / segments };
    const float fAngleStepOuter{ (outer_ang_max - outer_ang_min) / segments };

    const ImVec2& vTexUvWhitePixel{ ImGui::GetDrawListSharedData()->TexUvWhitePixel };
    // draw an arc for the current item
    const auto    drawList{ ImGui::GetWindowDrawList() };
    drawList->PrimReserve(segments * 6, (segments + 1) * 2);
    for(int iSeg{}; iSeg <= segments; ++iSeg) {
        const float fCosInner{ cosf(inner_ang_min + fAngleStepInner * iSeg) };
        const float fSinInner{ sinf(inner_ang_min + fAngleStepInner * iSeg) };
        const float fCosOuter{ cosf(outer_ang_min + fAngleStepOuter * iSeg) };
        const float fSinOuter{ sinf(outer_ang_min + fAngleStepOuter * iSeg) };

        if(iSeg < segments) {
            drawList->PrimWriteIdx(drawList->_VtxCurrentIdx + 0);
            drawList->PrimWriteIdx(drawList->_VtxCurrentIdx + 2);
            drawList->PrimWriteIdx(drawList->_VtxCurrentIdx + 1);
            drawList->PrimWriteIdx(drawList->_VtxCurrentIdx + 3);
            drawList->PrimWriteIdx(drawList->_VtxCurrentIdx + 2);
            drawList->PrimWriteIdx(drawList->_VtxCurrentIdx + 1);
        }

        // Interpolate between the start and end colors for inner vertex
        ImU32 colorInner{ bigLerp(color_start, color_end, 0) };
        Utils::Color::MultAlpha(colorInner, a_drawArgs.alphaMultiplier);
        drawList->PrimWriteVtx(ImVec2(center.x + fCosInner * radius_min, center.y + fSinInner * radius_min), vTexUvWhitePixel, colorInner);

        // Interpolate between the start and end colors for outer vertex
        ImU32 colorOuter{ bigLerp(color_start, color_end, 1) };
        Utils::Color::MultAlpha(colorOuter, a_drawArgs.alphaMultiplier);
        drawList->PrimWriteVtx(ImVec2(center.x + fCosOuter * radius_max, center.y + fSinOuter * radius_max), vTexUvWhitePixel, colorOuter);
    }
}

void Drawer::draw_circle_filled(const ImVec2& center, float radius, ImU32 color, int segments, const DrawArgs& a_drawArgs) {
    Utils::Color::MultAlpha(color, a_drawArgs.alphaMultiplier);
    ImGui::GetWindowDrawList()->AddCircleFilled(center, radius, color, segments);
}

void Drawer::draw_triangle_filled(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, ImU32 color, const DrawArgs& a_drawArgs) {
    Utils::Color::MultAlpha(color, a_drawArgs.alphaMultiplier);
    ImGui::GetWindowDrawList()->AddTriangleFilled(p1, p2, p3, color);
}

