#include "opengl.hpp"
#include <memory>

class Texture;

/**
 * @brief Create, manage, and automatically free a frame buffer
 */
class OglFramebuffer {
public:
    OglFramebuffer() { glGenFramebuffers(1, &m_handle); }
    ~OglFramebuffer() { glDeleteFramebuffers(1, &m_handle); }
    void SetColorAttachment(std::shared_ptr<Texture> tex = nullptr);
    void SetColorAttachmentInternal(GLuint tex_handle, int index);
    GLuint GlHandle() { return m_handle; }

private:
    std::shared_ptr<Texture> color_attachment = nullptr;
    GLuint m_handle = 0;
};
