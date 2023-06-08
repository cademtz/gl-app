#include "app.hpp"
#include "font.hpp"
#include "glm/ext/scalar_constants.hpp"
#include "platform.hpp"
#include <render/texture.hpp>
#include <render/gui/draw.hpp>
#include <glm/glm.hpp>
#include <cmath>

// Temporary includes for testing
#include "demostuff.hpp"
#include "render/gui/rendergui.hpp"
#include "render/sticks/draw.hpp"
#include "render/sticks/drawlist.hpp"
#include "render/sticks/rendersticks.hpp"
#include <chrono>
#include <string>
#include <vector>
#include <resources/resource.hpp>
#include <string_view>
#include <iostream>
#include <cstdlib>
#include <cstring>

Texture::Ptr default_tex;
gui::Draw draw;
gui::Draw::Font default_font;
std::unique_ptr<DemoStuff> demostuff = std::make_unique<DemoStuff>();

ClientTexture::Ptr make_checkers(TextureInfo&& info, uint32_t checker_size, glm::vec4 color1, glm::vec4 color2) {
    ClientTexture::Ptr tex = ClientTexture::Create(std::move(info));
    for (uint32_t y = 0; y < tex->GetHeight(); ++y) {
        for (uint32_t x = 0; x < tex->GetWidth(); ++x) {
            uint8_t* pixel = tex->GetPixel(x, y);
            
            bool checker = (x % (checker_size * 2) < checker_size)
                         ^ (y % (checker_size * 2) < checker_size);
            
            glm::vec4 selected_color = color1;
            if (!checker)
                selected_color = color2;
            
            for (uint32_t i = 0; i < tex->GetPixelStride(); ++i)
                pixel[i] = std::round(selected_color[i] * 255);
        }
    }
    return tex;
}

double oscillate(double a, double b, double freq_seconds, double offset = 0) {
    static const auto time_begin = std::chrono::high_resolution_clock::now();
    auto time_now = std::chrono::high_resolution_clock::now();

    using duration_type = std::chrono::nanoseconds;

    auto duration = std::chrono::duration_cast<duration_type>(time_now - time_begin);
    double duration_seconds = ((double)duration.count() / duration_type::period::den) * duration_type::period::num;
    duration_seconds += offset;

    double value_delta = b-a;
    double half_delta = value_delta/2;

    double result = -glm::cos(2. * glm::pi<double>() * duration_seconds / freq_seconds) * half_delta + half_delta;
    return result + a;
}

glm::vec3 crappy_hue_to_rgb(float hue) {
    constexpr float pi = glm::pi<float>();
    constexpr float freq = pi * 2;
    glm::vec3 rgb = {
        std::cos(freq * hue),
        std::cos(freq * hue + (2.f/3)*pi),
        std::cos(freq * hue + (4.f/3)*pi)
    };
    for (uint32_t i = 0; i < 3; ++i) {
        // normalize [0, 1]
        rgb[i] = rgb[i] / 2 + 0.5;
        //rgb[i] -= 0.25;
        //rgb[i] *= 1.f / 0.75f;
    }
    return rgb;
}

void App::OnSetup() {
    Platform::SetInputHandler(demostuff.get());

    Platform::AddRepeatingTask([] {
        Platform::PreRender();
        App::Render();
        Platform::PostRender();
        return true;
    });

    Platform::AddRepeatingTask([] {
        ClientTexture::Ptr chekrz = make_checkers(
            TextureInfo(TextureFormat::RGBA_8_32, 32, 32),
            8, glm::vec4(1, 0, 1, 1), glm::vec4(0, 0, 0, 1)
        );
        default_tex = Texture::Create(chekrz);
        return false;
    });

    Platform::AddRepeatingTask([] {
        default_font = draw.CreateFont(FontBakeConfig("Open_Sans/static/OpenSans-Regular.ttf", 32, 3));
        return false;
    });
}

void App::OnCleanup() {
}

void App::Render()
{
    const std::vector<glm::vec2>* points;
    static std::basic_string<uint32_t> text;
    int width, height;
    Platform::GetFrameBufferSize(&width, &height);
    
    text.clear();
    draw.Clear();

    // Clear screen with black rect
    draw.SetColor(0,0,0);
    draw.Rect(0, 0, width, height);

    points = &demostuff->GetPoints();
    demostuff->GetText(&text);
    
    // Draw all points
    draw.SetColor(1,1,1, 0.25);
    const double freq_seconds = 3;
    for (const glm::vec2& point : *points) {
        double offset = point.x / width * freq_seconds;
        glm::vec2 point_radii = glm::round(glm::vec2(oscillate(8, 32, freq_seconds, offset)));
        draw.TextureEllipse(default_tex, 64, point - point_radii, point_radii * 2.f);
    }

    // Draw all text
    draw.SetColor(0.6, 0.8, 1);
    draw.TextUnicode(default_font, glm::vec2(32, 70), text);

    size_t num_drawcalls = draw.GetDrawList().calls.size();
    draw.ResetColor();
    draw.TextAscii(default_font, glm::vec2(32, 50),
        std::to_string(num_drawcalls) + " draw calls\n" +
        std::to_string(points->size()) + " points"
    );

    static auto drawSticks = sticks::Draw();

    drawSticks.Clear();
    drawSticks.SetColor(1, 1, 1);
    drawSticks.Segment(
        sticks::Point{.pos=glm::vec2(0.2, 0.2), .cap=sticks::SegmentCap::CIRCLE, .rgba=glm::vec4(1.f)},
        sticks::Point{.pos=glm::vec2(-0.5, -0.5), .cap=sticks::SegmentCap::CIRCLE, .rgba=glm::vec4(1.f)}
    );
    
    static std::shared_ptr<gui::RenderGui> guiRenderer = gui::RenderGui::GetInstance();
    guiRenderer->SetScreenSize(width, height);
    guiRenderer->UploadDrawData(draw.GetDrawList());
    guiRenderer->Render();

    sticks::DrawList dlist;
    dlist.vertices.push_back(sticks::Vertex {
        0, 0, 0, 0, 1, 1, 1, 1
    });
    dlist.vertices.push_back(sticks::Vertex {
        0, 1, .5f, 0, 1, 0, 0, 1
    });
    dlist.vertices.push_back(sticks::Vertex {
        1, 1, 1, 1, 0, 1, 0, 1
    });

    dlist.indices.push_back(0);
    dlist.indices.push_back(1);
    dlist.indices.push_back(2);

    dlist.calls.push_back(sticks::DrawCall{
        0, 3, glm::mat3x3(1.f)
    });

    static auto stickRender = sticks::RenderSticks::GetInstance();
    stickRender->UploadDrawData(dlist);
    stickRender->Render();
}
