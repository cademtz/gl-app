#include "app.hpp"
#include <render/geometry.hpp>
#include <stb_truetype.h>
#include <chrono>

// Temporary includes for testing
#include <impl/render/opengl/opengl.hpp>
#include <impl/window/glfw/glfw.hpp>
#include <resources/resource.hpp>
#include <string_view>
#include <iostream>

static std::unique_ptr<CRenderGui> guiRenderer = nullptr;
static const Geometry2d base_geom = {
    // Vertices
    { //  x y       u v     r g b a
        { 100,100,    0,0,    1,0,0,1 },
        { 100,200,    0,10,   0,1,0,1 },
        { 200,200,    10,10,  0,0,1,1 },
        { 200,100,    10,0,   1,1,1,1 },
    },
    // Indices
    {
        0, 1, 2,    2, 3, 0,
    }
};

Geometry2d geom_buf;

void App::OnSetup() {
    guiRenderer->Init();
}

void App::OnCleanup() {
    guiRenderer = nullptr;
}

void App::Loop()
{
    // Printing only works in the loop function, so we must cause a flush here
    static bool firstRun = true;
    if (firstRun) {
        std::fflush(stdout);

        std::cout << "Attempting to load resource..." << std::endl;
        std::shared_ptr<CResource> resPtr = CResource::Load("resources/Open_Sans/README.txt");
        if (resPtr) {
            std::cout << "Resource contents:" << std::endl;
            std::cout << std::string_view(resPtr->Data(), resPtr->Length()) << std::endl;
        }
        else {
            std::cout << "[!] Resource failed to load" << std::endl;
        }

        firstRun = false;
    }

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

    shapes.SetAlign(PrimitiveBuilder2d::Align_Center);
    shapes.SetRotation(duration_seconds);
    shapes.SetColor(0, 1, 1);
    shapes.Rectangle(400, 400, 200, 50);
    shapes.SetColor(1, 0, 0);
    shapes.Ellipse(400, 400, 200, 50);

    guiRenderer->UploadGeometry(geom_buf);

    guiRenderer->Render();

    glfwSwapBuffers(impl::glfw::window);
}

void App::SetGuiRenderer(std::unique_ptr<CRenderGui>&& GuiRenderer) { guiRenderer = std::move(GuiRenderer); }
const std::unique_ptr<CRenderGui>& App::GetGuiRenderer() { return guiRenderer; }
