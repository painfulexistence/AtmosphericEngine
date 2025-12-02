#include "render_texture.hpp"
#include "console.hpp"
#include <stdexcept>

RenderTexture::RenderTexture(int width, int height, bool withDepth)
    : _width(width), _height(height), _withDepth(withDepth) {
    Create();
}

RenderTexture::RenderTexture(const Props& props)
    : _width(props.width),
      _height(props.height),
      _withDepth(props.withDepth),
      _withStencil(props.withStencil),
      _hdr(props.hdr),
      _filtered(props.filtered) {
    Create();
}

RenderTexture::~RenderTexture() {
    Destroy();
}

RenderTexture::RenderTexture(RenderTexture&& other) noexcept
    : _fbo(other._fbo),
      _colorTexture(other._colorTexture),
      _depthTexture(other._depthTexture),
      _depthStencilRBO(other._depthStencilRBO),
      _width(other._width),
      _height(other._height),
      _withDepth(other._withDepth),
      _withStencil(other._withStencil),
      _hdr(other._hdr),
      _filtered(other._filtered) {
    other._fbo = 0;
    other._colorTexture = 0;
    other._depthTexture = 0;
    other._depthStencilRBO = 0;
}

RenderTexture& RenderTexture::operator=(RenderTexture&& other) noexcept {
    if (this != &other) {
        Destroy();

        _fbo = other._fbo;
        _colorTexture = other._colorTexture;
        _depthTexture = other._depthTexture;
        _depthStencilRBO = other._depthStencilRBO;
        _width = other._width;
        _height = other._height;
        _withDepth = other._withDepth;
        _withStencil = other._withStencil;
        _hdr = other._hdr;
        _filtered = other._filtered;

        other._fbo = 0;
        other._colorTexture = 0;
        other._depthTexture = 0;
        other._depthStencilRBO = 0;
    }
    return *this;
}

void RenderTexture::Create() {
    // Create framebuffer
    glGenFramebuffers(1, &_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

    // Create color texture
    glGenTextures(1, &_colorTexture);
    glBindTexture(GL_TEXTURE_2D, _colorTexture);

    GLenum internalFormat = _hdr ? GL_RGBA16F : GL_RGBA8;
    GLenum format = GL_RGBA;
    GLenum type = _hdr ? GL_FLOAT : GL_UNSIGNED_BYTE;

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, _width, _height, 0, format, type, nullptr);

    GLenum filterMode = _filtered ? GL_LINEAR : GL_NEAREST;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _colorTexture, 0);

    // Create depth/stencil attachment if requested
    if (_withDepth) {
        if (_withStencil) {
            // Use renderbuffer for depth+stencil
            glGenRenderbuffers(1, &_depthStencilRBO);
            glBindRenderbuffer(GL_RENDERBUFFER, _depthStencilRBO);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, _width, _height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _depthStencilRBO);
        } else {
            // Use texture for depth only (allows sampling)
            glGenTextures(1, &_depthTexture);
            glBindTexture(GL_TEXTURE_2D, _depthTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, _width, _height, 0,
                         GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthTexture, 0);
        }
    }

    // Check framebuffer completeness
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        Console::Get()->Error("RenderTexture: Framebuffer incomplete!");
        Destroy();
    }

    // Restore default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderTexture::Destroy() {
    if (_depthStencilRBO != 0) {
        glDeleteRenderbuffers(1, &_depthStencilRBO);
        _depthStencilRBO = 0;
    }
    if (_depthTexture != 0) {
        glDeleteTextures(1, &_depthTexture);
        _depthTexture = 0;
    }
    if (_colorTexture != 0) {
        glDeleteTextures(1, &_colorTexture);
        _colorTexture = 0;
    }
    if (_fbo != 0) {
        glDeleteFramebuffers(1, &_fbo);
        _fbo = 0;
    }
}

void RenderTexture::Begin() {
    // Save current state
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &_prevFBO);
    glGetIntegerv(GL_VIEWPORT, _prevViewport);

    // Bind this render texture
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    glViewport(0, 0, _width, _height);
}

void RenderTexture::End() {
    // Restore previous state
    glBindFramebuffer(GL_FRAMEBUFFER, _prevFBO);
    glViewport(_prevViewport[0], _prevViewport[1], _prevViewport[2], _prevViewport[3]);
}

void RenderTexture::Clear(const glm::vec4& color) {
    GLint prevFBO;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFBO);

    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    glClearColor(color.r, color.g, color.b, color.a);

    GLbitfield clearMask = GL_COLOR_BUFFER_BIT;
    if (_withDepth) {
        clearMask |= GL_DEPTH_BUFFER_BIT;
        if (_withStencil) {
            clearMask |= GL_STENCIL_BUFFER_BIT;
        }
    }
    glClear(clearMask);

    glBindFramebuffer(GL_FRAMEBUFFER, prevFBO);
}

void RenderTexture::Resize(int width, int height) {
    if (width == _width && height == _height) return;

    _width = width;
    _height = height;

    Destroy();
    Create();
}
