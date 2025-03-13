#pragma once
#include <vector>
#include <cstdint>
#include <glm/vec4.hpp>
#include <glm/glm.hpp>
#include <cmath>
#include "forward.hpp"
#include "opengl/forward.hpp"

namespace Render2d {
    static const glm::vec4 NO_CLIP = glm::vec4(NAN);

    struct Vertex
    {
        float x, y;
        float u, v;
        float r, g, b, a;
    };

    /**
     * @brief Parameters of a single render call.
     * Calls are grouped under a @ref DrawList, which contains all vertices and indices.
     */
    struct DrawCall {
        /**
         * @brief Offset into the index buffer
         */
        uint32_t index_offset = 0;
        /**
         * @brief Number of indices to use
         */
        uint32_t index_count = 0;
        /**
         * @brief A rectangle in `{ x, y, w, h }` format
         */
        glm::vec4 clip_rect = NO_CLIP;
        TexturePtr texture = nullptr;
        OglProgramPtr program = nullptr;
    };
    
    /**
     * @brief A list of draw calls using the same vertex and index buffer.
     */
    struct DrawList {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<DrawCall> calls;
    };
}