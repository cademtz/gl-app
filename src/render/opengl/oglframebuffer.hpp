#include "opengl.hpp"
#include <render/forward.hpp>

class Texture;

/**
 * @brief Create, manage, and automatically free a frame buffer
 */
class OglFramebuffer {
public:
    OglFramebuffer() { glGenFramebuffers(1, &m_handle); }
    ~OglFramebuffer() { glDeleteFramebuffers(1, &m_handle); }
    void SetColorAttachment(TexturePtr tex = nullptr);
    void SetColorAttachmentInternal(GLuint tex_handle, int index);
    GLuint GlHandle() { return m_handle; }

private:
    OglFramebuffer(const OglFramebuffer&) = delete;
    
    TexturePtr color_attachment = nullptr;
    GLuint m_handle = 0;
};
