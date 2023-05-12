#include <render/texture.hpp>
#include <cassert>
#include "opengl.hpp"

class COpenGLTexture : public Texture
{
public:
    COpenGLTexture(TextureInfo&& info, GLuint id)
        : Texture(std::move(info)), m_id(id) {}
    
    ~COpenGLTexture() {
        glDeleteTextures(1, &m_id);
    }

    void Write(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void* data) override {
        assert(0 && "Not implemented");
    }

    GLuint GetId() const { return m_id; }

private:
    GLuint m_id;
};