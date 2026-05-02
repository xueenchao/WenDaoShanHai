/**
 * MemoryTexture.cpp - 从内存加载的纹理类实现
 */

#include "MemoryTexture.h"
#include "Renderer.h"
#include "Log.h"
#include <SDL3_image/SDL_image.h>

// ==================== 构造与析构 ====================

MemoryTexture::MemoryTexture()
    : mTexture(nullptr)
    , mWidth(0)
    , mHeight(0)
{
}

MemoryTexture::~MemoryTexture()
{
    destroy();
}

// ==================== 移动语义 ====================

MemoryTexture::MemoryTexture(MemoryTexture&& other) noexcept
    : mTexture(other.mTexture)
    , mWidth(other.mWidth)
    , mHeight(other.mHeight)
{
    other.mTexture = nullptr;
    other.mWidth = 0;
    other.mHeight = 0;
}

MemoryTexture& MemoryTexture::operator=(MemoryTexture&& other) noexcept
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

// ==================== 内存加载 ====================

bool MemoryTexture::loadFromMemory(Renderer& renderer, const void* data, size_t dataSize)
{
    destroy();

    SDL_Renderer* rawRenderer = renderer.getRawRenderer();
    if (rawRenderer == nullptr) {
        LOG_ERROR("MemoryTexture 加载失败: 渲染器尚未创建");
        return false;
    }

    if (data == nullptr || dataSize == 0) {
        LOG_ERROR("MemoryTexture 加载失败: 数据为空");
        return false;
    }

    SDL_IOStream* io = SDL_IOFromConstMem(data, dataSize);
    if (io == nullptr) {
        LOG_ERROR("MemoryTexture 创建IOStream失败: %s", SDL_GetError());
        return false;
    }

    mTexture = IMG_LoadTexture_IO(rawRenderer, io, true);

    if (mTexture == nullptr) {
        LOG_ERROR("MemoryTexture 加载失败: %s", SDL_GetError());
        return false;
    }

    float w = 0.0f, h = 0.0f;
    SDL_GetTextureSize(mTexture, &w, &h);
    mWidth = static_cast<int>(w);
    mHeight = static_cast<int>(h);

    return true;
}

// ==================== 实现基类接口 ====================

SDL_Texture* MemoryTexture::getRawTexture() const
{
    return mTexture;
}

int MemoryTexture::getWidth() const
{
    return mWidth;
}

int MemoryTexture::getHeight() const
{
    return mHeight;
}

bool MemoryTexture::setColorMod(Uint8 r, Uint8 g, Uint8 b)
{
    if (mTexture == nullptr) {
        return false;
    }
    return SDL_SetTextureColorMod(mTexture, r, g, b);
}

bool MemoryTexture::setAlphaMod(Uint8 alpha)
{
    if (mTexture == nullptr) {
        return false;
    }
    return SDL_SetTextureAlphaMod(mTexture, alpha);
}

Uint8 MemoryTexture::getAlphaMod() const
{
    if (mTexture == nullptr) {
        return 255;
    }
    Uint8 alpha = 255;
    SDL_GetTextureAlphaMod(mTexture, &alpha);
    return alpha;
}

bool MemoryTexture::setBlendMode(SDL_BlendMode blendMode)
{
    if (mTexture == nullptr) {
        return false;
    }
    return SDL_SetTextureBlendMode(mTexture, blendMode);
}

bool MemoryTexture::isValid() const
{
    return mTexture != nullptr;
}

void MemoryTexture::destroy()
{
    if (mTexture != nullptr) {
        SDL_DestroyTexture(mTexture);
        mTexture = nullptr;
    }
    mWidth = 0;
    mHeight = 0;
}
