#include <render/texture.hpp>
#include <cassert>
#include "opengl.hpp"

class OglTexture : public Texture
{
public:
    OglTexture(TextureInfo&& info, GLuint id)
        : Texture(std::move(info)), m_handle(id) {}
    ~OglTexture() { glDeleteTextures(1, &m_handle); }
    void Resize(uint32_t width, uint32_t height) override;
    void Write(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void* data) override;
    void ClearColor(float r, float g, float b, float a) override;
    GLuint GlHandle() const { return m_handle; }

private:
    const GLuint m_handle;
};