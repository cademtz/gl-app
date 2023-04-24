#pragma once
#include <stb_rect_pack.h>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <vector>
#include <cassert>

/**
 * @brief Tightly pack rectangles into a small space
 */
class CRectPacker
{
public:
    using Rect = stbrp_rect;

    /**
     * @brief Create a rectangle packer with a maximum container size
     * @param max_w Maximum width
     * @param max_h Maximum height
     */
    CRectPacker(uint32_t max_w, uint32_t max_h)
        : m_max_w(max_w), m_max_h(max_h) {}
    
    CRectPacker() {}

    void GetMaxSize(uint32_t* out_max_w, uint32_t* out_max_h) const {
        *out_max_h = m_max_w, *out_max_h = m_max_h;
    }
    void SetMaxSize(uint32_t max_w, uint32_t max_h) {
        m_max_w = max_w, m_max_h = max_h;
    }
    
    bool Pack();

    bool IsPacked() const { return m_is_packed; }

    /**
     * @brief Get final dimensions of the rectangle after successful packing
     */
    void GetPackedSize(uint32_t* out_packed_w, uint32_t* out_packed_h) const {
        *out_packed_w = m_packed_w, *out_packed_h = m_packed_h;
    }

    const std::vector<Rect>& GetRects() const { return m_rects; }

    void GetRect(uint32_t rect_id, Rect* out_rect) const {
        *out_rect = m_rects[(size_t)rect_id];
    }

    /**
     * @brief Add a new rectangle to the packer
     * @return A rectangle ID
     */
    uint32_t AddRect(uint32_t width, uint32_t height);

    /**
     * @brief Remove all rectangles
     */
    void Clear();

private:
    bool m_is_packed = false;
    uint32_t m_packed_w = 0, m_packed_h = 0;
    uint32_t m_max_w = ~(uint32_t)0, m_max_h = ~(uint32_t)0;
    std::vector<Rect> m_rects;
};

/**
 * @brief Takes in a number of inputs
 */
class CBakeConfig
{
public:
    using Input = std::function<void()>;
    uint32_t NewRect();

private:
    std::unordered_set<uint32_t> m_ids;
};

class CBakeResult
{
public:
    inline const stbrp_rect* GetRect(uint32_t id) const
    {
        auto it = m_rects.find(id);
        if (it == m_rects.cend())
            return nullptr;
        return &it->second;
    }

private:
    std::unordered_map<uint32_t, stbrp_rect> m_rects;
};