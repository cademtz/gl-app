#include "app.hpp"
#include "font.hpp"
#include "platform.hpp"
#include "render/bake.hpp"
#include "render/texture.hpp"
#include <render/geometry.hpp>
#include <stb_truetype.h>
#include <chrono>
#include <cmath>

// Temporary includes for testing
#include <resources/resource.hpp>
#include <string_view>
#include <iostream>
#include <cstdlib>
#include <cstring>

static std::unique_ptr<CRenderGui> guiRenderer = nullptr;
static const Geometry2d base_geom = {
    // Vertices
    { //  x y       u v     r g b a
        { 100,100,    0,0,  1,1,1,1 },
        { 200,100,    1,0,  1,1,1,1 },
        { 200,200,    1,1,  1,1,1,1 },
        { 100,200,    0,1,  1,1,1,1 },
    },
    // Indices
    {
        0, 1, 2,    2, 3, 0,
    }
};

Geometry2d geom_buf;
CTexture::Ptr default_tex = nullptr;

void App::OnSetup() {
    guiRenderer->Init();

    Platform::AddRepeatingTask(
        [] {
            Platform::PreRender();
            App::Render();
            Platform::PostRender();
            return true;
        }
    );

    Platform::AddRepeatingTask(
        [] {
            static int count = 0;
            ++count;
            std::cout << "Ran " << count << " times" << std::endl;

            CResource::Ptr ttf_res = CResource::LoadSynchronous("Open_Sans/static/OpenSans/OpenSans-Regular.ttf");
            if (!ttf_res)
                assert(0 && "Failed to load TTF resource");
            std::optional<CTrueType> ttf = CTrueType::FromTrueType(ttf_res);
            if (!ttf)
                assert(0 && "Failed to parse TTF");
            
            uint32_t glyph = ttf->FindGlyphId('R');
            float scale = ttf->ScaleForPixelHeight(90);
            
            int32_t x0, y0, x1, y1;
            ttf->GetGlyphBitmapBox(
                glyph,
                scale, scale,
                0, 0,
                &x0, &y0, &x1, &y1
            );
        
            uint32_t glyph_w = x1 - x0;
            uint32_t glyph_h = y1 - y0;
            std::cout << "glyph_id: " << glyph << " glyph_w: " << glyph_w << " glyph_h: " << glyph_h << std::endl;
            assert((glyph_h & (1 << 31)) == 0);
        
            CClientTexture::Ptr test = CClientTexture::Create(
                CTextureInfo(TextureFormat::A_8_8, glyph_w, glyph_h)
            );

            std::cout << "Allocated the glyph bitmap" << test->GetWidth() << test->GetHeight() << std::endl;
        
            std::memset(test->GetData(), 0xFF, test->GetRowStride() * test->GetHeight());

            /*double max_dst = std::sqrt((double)(test->GetWidth() * test->GetWidth() + test->GetHeight() * test->GetHeight()));
            for (uint32_t y = 0; y < test->GetHeight(); ++y) {
                for (uint32_t x = 0; x < test->GetWidth(); ++x) {
                    uint8_t* rgba = &test->GetData()[y * test->GetRowStride() + x * test->GetPixelStride()];
                    double dst = std::sqrt((double)(x*x + y*y));
                    //rgba[3] = (uint8_t)std::round((dst / max_dst) * 255);
                    rgba[2] = 0;
                    rgba[0] = ((float)x / test->GetWidth()) * 255;
                    rgba[1] = ((float)y / test->GetHeight()) * 255;
                    //rgba[3] = (((col % 32 > 16) ^ (row % 32 > 16)) == 1) * 255;
                }
            }*/

            std::cout << "Cleared the glyph bitmap" << std::endl;

            ttf->MakeGlyphBitmap(
                glyph,
                scale, scale, 0, 0,
                test->GetData(), test->GetWidth(), test->GetHeight(), 0
            );

            std::cout << "Rendered the glyph" << std::endl;
        
            default_tex = CTexture::Create(test);
            return false;
        }
    );
}

void App::OnCleanup() {
    guiRenderer = nullptr;
}

void App::Render()
{
    int width, height;
    Platform::GetFrameBufferSize(&width, &height);
    guiRenderer->SetScreenSize(width, height);

    geom_buf.vertices.clear();
    geom_buf.indices.clear();

    PrimitiveBuilder2d shapes(geom_buf);

    // Add a black rectangle covering the screen to overwrite the old buffer
    shapes.SetColor(0, 0, 0);
    shapes.Rectangle(0, 0, width, height);

    // Append some geometry data manually
    {
        int vertex_base = geom_buf.vertices.size();
        geom_buf.vertices.insert(geom_buf.vertices.end(), base_geom.vertices.begin(), base_geom.vertices.end());
        for (vertexId_t id : base_geom.indices)
            geom_buf.indices.push_back(id + vertex_base);
    }

    static const auto time_begin = std::chrono::high_resolution_clock::now();
    auto time_now = std::chrono::high_resolution_clock::now();

    using duration_type = std::chrono::nanoseconds;

    auto duration = std::chrono::duration_cast<duration_type>(time_now - time_begin);
    double duration_seconds = ((double)duration.count() / duration_type::period::den) * duration_type::period::num;
    duration_seconds = 0;

    shapes.SetAlign(PrimitiveBuilder2d::Align_Center);
    shapes.SetRotation(duration_seconds);
    shapes.SetColor(0, 1, 1);
    shapes.Rectangle(400, 400, 200, 50);
    shapes.SetColor(1, 0, 0);
    shapes.Ellipse(400, 400, 200, 50);
    shapes.ClearTransform();

    geom_buf.texture = default_tex;

    guiRenderer->UploadGeometry(geom_buf);

    guiRenderer->Render();
}

void App::SetGuiRenderer(std::unique_ptr<CRenderGui>&& GuiRenderer) { guiRenderer = std::move(GuiRenderer); }
const std::unique_ptr<CRenderGui>& App::GetGuiRenderer() { return guiRenderer; }
