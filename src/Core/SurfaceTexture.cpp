/**
 * SurfaceTexture.cpp - 从 SDL_Surface 转换的纹理类实现
 */

#include "SurfaceTexture.h"
#include "Renderer.h"
#include "Log.h"

// ==================== 构造与析构 ====================

SurfaceTexture::SurfaceTexture()
    : mTexture(nullptr)
    , mWidth(0)
    , mHeight(0)
{
}

SurfaceTexture::~SurfaceTexture()
{
    destroy();
}

// ==================== 移动语义 ====================

SurfaceTexture::SurfaceTexture(SurfaceTexture&& other) noexcept
    : mTexture(other.mTexture)
    , mWidth(other.mWidth)
    , mHeight(other.mHeight)
{
    other.mTexture = nullptr;
    other.mWidth = 0;
    other.mHeight = 0;
}

SurfaceTexture& SurfaceTexture::operator=(SurfaceTexture&& other) noexcept
{
    if (this != &other) {
        destroy();

        mTexture = other.mTexture;
        mWidth = other.mWidth;
        mHeight = other.mHeight;

        other.mTexture = nullptr;
        other.mWidth = 0;
        other.mHeight = 0;
    }
    return *this;
}

// ==================== 从 Surface 转换 ====================

bool SurfaceTexture::createFromSurface(Renderer& renderer, SDL_Surface* surface)
{
    destroy();

    SDL_Renderer* rawRenderer = renderer.getRawRenderer();
    if (rawRenderer == nullptr || surface == nullptr) {
        LOG_ERROR("SurfaceTexture 创建失败: 渲染器或Surface无效");
        return false;
    }

    mTexture = SDL_CreateTextureFromSurface(rawRenderer, surface);

    if (mTexture == nullptr) {
        LOG_ERROR("SurfaceTexture 创建失败: %s", SDL_GetError());
        return false;
    }

    mWidth = surface->w;
    mHeight = surface->h;

    return true;
}

// ==================== 实现基类接口 ====================

SDL_Texture* SurfaceTexture::getRawTexture() const
{
    return mTexture;
}

int SurfaceTexture::getWidth() const
{
    return mWidth;
}

int SurfaceTexture::getHeight() const
{
    return mHeight;
}

bool SurfaceTexture::setColorMod(Uint8 r, Uint8 g, Uint8 b)
{
    if (mTexture == nullptr) {
        return false;
    }
    return SDL_SetTextureColorMod(mTexture, r, g, b);
}

bool SurfaceTexture::setAlphaMod(Uint8 alpha)
{
    if (mTexture == nullptr) {
        return false;
    }
    return SDL_SetTextureAlphaMod(mTexture, alpha);
}

Uint8 SurfaceTexture::getAlphaMod() const
{
    if (mTexture == nullptr) {
        return 255;
    }
    Uint8 alpha = 255;
    SDL_GetTextureAlphaMod(mTexture, &alpha);
    return alpha;
}

bool SurfaceTexture::setBlendMode(SDL_BlendMode blendMode)
{
    if (mTexture == nullptr) {
        return false;
    }
    return SDL_SetTextureBlendMode(mTexture, blendMode);
}

bool SurfaceTexture::isValid() const
{
    return mTexture != nullptr;
}

void SurfaceTexture::destroy()
{
    if (mTexture != nullptr) {
        SDL_DestroyTexture(mTexture);
        mTexture = nullptr;
    }
    mWidth = 0;
    mHeight = 0;
}
