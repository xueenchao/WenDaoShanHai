/**
 * BlankTexture.cpp - 空白纹理类实现
 */

#include "BlankTexture.h"
#include "Renderer.h"
#include "Log.h"

// ==================== 构造与析构 ====================

BlankTexture::BlankTexture()
    : mTexture(nullptr)
    , mWidth(0)
    , mHeight(0)
    , mAccess(SDL_TEXTUREACCESS_TARGET)
{
}

BlankTexture::~BlankTexture()
{
    destroy();
}

// ==================== 移动语义 ====================

BlankTexture::BlankTexture(BlankTexture&& other) noexcept
    : mTexture(other.mTexture)
    , mWidth(other.mWidth)
    , mHeight(other.mHeight)
    , mAccess(other.mAccess)
{
    other.mTexture = nullptr;
    other.mWidth = 0;
    other.mHeight = 0;
    other.mAccess = SDL_TEXTUREACCESS_TARGET;
}

BlankTexture& BlankTexture::operator=(BlankTexture&& other) noexcept
{
    if (this != &other) {
        destroy();

        mTexture = other.mTexture;
        mWidth = other.mWidth;
        mHeight = other.mHeight;
        mAccess = other.mAccess;

        other.mTexture = nullptr;
        other.mWidth = 0;
        other.mHeight = 0;
        other.mAccess = SDL_TEXTUREACCESS_TARGET;
    }
    return *this;
}

// ==================== 创建空白纹理 ====================

bool BlankTexture::createBlank(Renderer& renderer, int width, int height,
                               SDL_TextureAccess access)
{
    destroy();

    SDL_Renderer* rawRenderer = renderer.getRawRenderer();
    if (rawRenderer == nullptr) {
        LOG_ERROR("BlankTexture 创建失败: 渲染器尚未创建");
        return false;
    }

    mTexture = SDL_CreateTexture(rawRenderer, SDL_PIXELFORMAT_RGBA8888,
                                 access, width, height);

    if (mTexture == nullptr) {
        LOG_ERROR("BlankTexture 创建失败: %s", SDL_GetError());
        return false;
    }

    mWidth = width;
    mHeight = height;
    mAccess = access;

    if (access == SDL_TEXTUREACCESS_TARGET) {
        SDL_SetTextureBlendMode(mTexture, SDL_BLENDMODE_BLEND);
    }

    return true;
}

bool BlankTexture::isRenderTarget() const
{
    return mAccess == SDL_TEXTUREACCESS_TARGET;
}

// ==================== 实现基类接口 ====================

SDL_Texture* BlankTexture::getRawTexture() const
{
    return mTexture;
}

int BlankTexture::getWidth() const
{
    return mWidth;
}

int BlankTexture::getHeight() const
{
    return mHeight;
}

bool BlankTexture::setColorMod(Uint8 r, Uint8 g, Uint8 b)
{
    if (mTexture == nullptr) {
        return false;
    }
    return SDL_SetTextureColorMod(mTexture, r, g, b);
}

bool BlankTexture::setAlphaMod(Uint8 alpha)
{
    if (mTexture == nullptr) {
        return false;
    }
    return SDL_SetTextureAlphaMod(mTexture, alpha);
}

Uint8 BlankTexture::getAlphaMod() const
{
    if (mTexture == nullptr) {
        return 255;
    }
    Uint8 alpha = 255;
    SDL_GetTextureAlphaMod(mTexture, &alpha);
    return alpha;
}

bool BlankTexture::setBlendMode(SDL_BlendMode blendMode)
{
    if (mTexture == nullptr) {
        return false;
    }
    return SDL_SetTextureBlendMode(mTexture, blendMode);
}

bool BlankTexture::isValid() const
{
    return mTexture != nullptr;
}

void BlankTexture::destroy()
{
    if (mTexture != nullptr) {
        SDL_DestroyTexture(mTexture);
        mTexture = nullptr;
    }
    mWidth = 0;
    mHeight = 0;
    mAccess = SDL_TEXTUREACCESS_TARGET;
}
