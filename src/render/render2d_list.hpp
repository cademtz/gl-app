#pragma once
#include <vector>
#include <cstdint>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <cmath>
#include "forward.hpp"
#include "opengl/forward.hpp"
#include "opengl/opengl.hpp"

namespace Render2d {
    static const glm::vec4 NO_CLIP = glm::vec4(NAN);
    template <class>
    constexpr bool _dependent_false = false;

    enum class ShaderParamType : uint8_t {
        INT, FLOAT, VEC2, VEC3, VEC4, MAT3X3, MAT4X4
    };

    constexpr ShaderParamType ToShaderParamType(int*)         { return ShaderParamType::INT; }
    constexpr ShaderParamType ToShaderParamType(float*)       { return ShaderParamType::FLOAT; }
    constexpr ShaderParamType ToShaderParamType(glm::vec2*)   { return ShaderParamType::VEC2; }
    constexpr ShaderParamType ToShaderParamType(glm::vec3*)   { return ShaderParamType::VEC3; }
    constexpr ShaderParamType ToShaderParamType(glm::vec4*)   { return ShaderParamType::VEC4; }
    constexpr ShaderParamType ToShaderParamType(glm::mat3x3*) { return ShaderParamType::MAT3X3; }
    constexpr ShaderParamType ToShaderParamType(glm::mat4x4*) { return ShaderParamType::MAT4X4; }
    
    struct Vertex
    {
        float x, y;
        float u, v;
        float r, g, b, a;
    };

    /// @details This struct is separate so we can check for equality to split/batch calls
    struct DrawCallParams {
        /// @brief A rectangle in `{ x, y, w, h }` format
        glm::vec4 clip = NO_CLIP;
        TexturePtr texture = nullptr;
        /// @brief Use `nullptr` for the default program
        OglProgramPtr program = nullptr;
        glm::mat4 transform{1.f};

        bool operator==(const DrawCallParams& other) const = default;
        bool operator!=(const DrawCallParams& other) const = default;
    };

    struct ShaderParam {
        GLint id;
        ShaderParamType type;
        /// Index into drawlist's storage
        size_t index;
    };

    struct ShaderParamList {
        std::vector<ShaderParam> items;
        std::vector<int>         sp_int;
        std::vector<float>       sp_float;
        std::vector<glm::vec2>   sp_vec2;
        std::vector<glm::vec3>   sp_vec3;
        std::vector<glm::vec4>   sp_vec4;
        std::vector<glm::mat3x3> sp_mat3x3;
        std::vector<glm::mat4x4> sp_mat4x4;

        void Clear() {
            items.clear();
            sp_int.clear();
            sp_float.clear();
            sp_vec2.clear();
            sp_vec3.clear();
            sp_vec4.clear();
            sp_mat3x3.clear();
            sp_mat4x4.clear();
        }

        std::vector<int>& _GetVec(int*) { return sp_int; }
        std::vector<float>& _GetVec(float*) { return sp_float; }
        std::vector<glm::vec2>& _GetVec(glm::vec2*) { return sp_vec2; }
        std::vector<glm::vec3>& _GetVec(glm::vec3*) { return sp_vec3; }
        std::vector<glm::vec4>& _GetVec(glm::vec4*) { return sp_vec4; }
        std::vector<glm::mat3x3>& _GetVec(glm::mat3x3*) { return sp_mat3x3; }
        std::vector<glm::mat4x4>& _GetVec(glm::mat4x4*) { return sp_mat4x4; }

        template <class T>
        void Set(GLint id, const T& value) {
            ShaderParamType type = ToShaderParamType((T*)nullptr);
            std::vector<T>& vec = _GetVec((T*)nullptr);
            vec.push_back(value);
            items.emplace_back(id, type, vec.size()-1);
        }
    };

    /**
     * @brief Parameters of a single render call.
     * Calls are grouped under a @ref DrawList, which contains all vertices and indices.
     */
    struct DrawCall {
        /// @brief Offset into the index buffer
        uint32_t index_offset = 0;
        /// @brief Number of indices
        uint32_t index_count = 0;
        /// @brief Offset into the shader param buffer
        uint32_t sp_offset = 0;
        /// @brief Number of shader params
        uint32_t sp_count = 0;
        DrawCallParams params;
    };
    
    /// @brief A list of draw calls using the same vertex and index buffer.
    struct DrawList {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<DrawCall> calls;
        ShaderParamList shader_params;
    };
}