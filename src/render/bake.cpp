#include "stb_rect_pack.h"
#define STB_RECT_PACK_IMPLEMENTATION
#include "bake.hpp"
#include <vector>
#include <cmath>

// Debugging
#include <iostream>
#include <chrono>

uint32_t RectPacker::AddRect(uint32_t width, uint32_t height)
{
    Rect rect = { 0 };
    rect.x = -1, rect.y = -1;
    rect.w = width, rect.h = height;

    m_is_packed = false;
    m_rects.emplace_back(std::move(rect));
    return (uint32_t)m_rects.size() - 1;
}

void RectPacker::Clear() {
    m_is_packed = false;
    m_rects.clear();
}

bool RectPacker::Pack()
{
    // Constants
    const float padding_factor = 1.2;
    const float growth_factor = 1.2;
    const size_t max_iters = 1024;

    auto time_begin = std::chrono::high_resolution_clock::now();

    uint32_t target_w = 0, target_h = 0;

    // Make a decent guess for the minimum size of the rectangle
    {
        uint32_t greatest_w = 0, greatest_h = 0;
        uint32_t total_pixels = 0;
        for (const auto& rect : m_rects)
        {
            total_pixels += rect.w * rect.h;
            if (rect.w > greatest_w)
                greatest_w = rect.w;
            if (rect.h > greatest_h)
                greatest_h = rect.h;
        }
        float square = std::sqrt(total_pixels);
        float ratio = (float)target_w / target_h;

        std::cout << "total px: " << target_w * target_h << std::endl;
        std::cout << "square: " << square << std::endl;
        std::cout << "ratio: " << ratio << std::endl;

        target_w = square;
        target_h = square;

        if (target_w < greatest_w)
            target_w = greatest_w;
        if (target_h < greatest_h)
            target_h = greatest_h;
    }

    // Padding
    target_w *= padding_factor, target_h *= padding_factor;

    std::cout << "target_w: " << target_w << std::endl;
    std::cout << "target_h: " << target_h << std::endl;
    
    // Clamp to max heights
    if (target_w > m_max_w)
        target_w = m_max_w;
    if (target_h > m_max_h)
        target_h = m_max_h;

    stbrp_context ctx;    
    std::vector<stbrp_node> nodes(target_w);

    stbrp_init_target(&ctx, target_w, target_h, &nodes[0], nodes.size());
    bool result = stbrp_pack_rects(&ctx, &m_rects[0], m_rects.size()) != 0;

    size_t iteration = 1;
    while (!result) {
        if (iteration >= max_iters)
            break;

        uint32_t new_w = target_w * growth_factor;
        uint32_t new_h = target_h * growth_factor;

        // Clamp to max heights
        if (new_w > m_max_w)
            new_w = m_max_w;
        if (new_h > m_max_h)
            new_h = m_max_h;

        if (new_w == target_w && new_h == target_h)
            break; // Maximum size was already tried. The rectangle cannot be packed.

        target_w = new_w;
        target_h = new_h;
        
        nodes.resize(target_w);
        stbrp_init_target(&ctx, target_w, target_h, &nodes[0], nodes.size());
        result = stbrp_pack_rects(&ctx, &m_rects[0], m_rects.size()) != 0;
        ++iteration;
    }

    m_packed_w = 0, m_packed_h = 0;
    for (const auto& rect : m_rects) {
        if (rect.x + rect.w > m_packed_w)
            m_packed_w = rect.x + rect.w;
        if (rect.y + rect.h > m_packed_h)
            m_packed_h = rect.y + rect.h;
    }
    m_is_packed = result;

    auto time_now = std::chrono::high_resolution_clock::now();

    using duration_type = std::chrono::milliseconds;

    auto duration = std::chrono::duration_cast<duration_type>(time_now - time_begin);
    double duration_ms = ((double)duration.count() / duration_type::period::den) * duration_type::period::num;

    std::cout << "packer tried " << iteration << " iterations" << std::endl;
    std::cout << "m_is_packed: " << m_is_packed << std::endl;

    if (!result)
        return false;

    std::cout << "Ended with " << target_w << "x" << target_h << " units" << std::endl;
    std::cout << "Trimmed to " << m_packed_w << "x" << m_packed_h << " units" << std::endl;
    std::cout << "Took " << duration_ms << " ms" << std::endl;
    std::cout << "Extra memory used: " << (float)(nodes.size() * sizeof(nodes[0])) / 1024 << " KB" << std::endl;
    return true;
}