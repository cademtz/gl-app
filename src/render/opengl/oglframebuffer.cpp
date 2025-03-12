#include "oglframebuffer.hpp"
#include "ogltexture.hpp"
#include <stdio.h>
#include <assert.h>

void OglFramebuffer::SetColorAttachment(std::shared_ptr<Texture> tex) {
    GLuint tex_handle = 0;
    if (tex)
        tex_handle = std::reinterpret_pointer_cast<OglTexture>(tex)->GlHandle();
    
    SetColorAttachmentInternal(tex_handle, 0);
}

void OglFramebuffer::SetColorAttachmentInternal(GLuint tex_handle, int index) {
    glBindFramebuffer(GL_FRAMEBUFFER, m_handle);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_2D, tex_handle, 0);

    if (tex_handle) {
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            printf("glCheckFramebufferStatus: ");
            switch (status) {
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: printf("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT"); break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: printf("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"); break;
            case GL_FRAMEBUFFER_UNSUPPORTED: printf("GL_FRAMEBUFFER_UNSUPPORTED"); break;
            default: printf("unknown (%X)", status);
            }
            printf("\n");
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

