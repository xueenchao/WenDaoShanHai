/**
 * FileTexture.cpp - 从文件加载的纹理类实现
 */

#include "FileTexture.h"
#include "Renderer.h"
#include "Log.h"
#include <SDL3_image/SDL_image.h>

// ==================== 构造与析构 ====================

FileTexture::FileTexture()
    : mTexture(nullptr)
    , mWidth(0)
    , mHeight(0)
{
}

FileTexture::~FileTexture()
{
    destroy();
}

// ==================== 移动语义 ====================

FileTexture::FileTexture(FileTexture&& other) noexcept
    : mTexture(other.mTexture)
    , mWidth(other.mWidth)
    , mHeight(other.mHeight)
    , mFilePath(std::move(other.mFilePath))
{
    other.mTexture = nullptr;
    other.mWidth = 0;
    other.mHeight = 0;
}

FileTexture& FileTexture::operator=(FileTexture&& other) noexcept
{
    if (this != &other) {
        destroy();

        mTexture = other.mTexture;
        mWidth = other.mWidth;
        mHeight = other.mHeight;
        mFilePath = std::move(other.mFilePath);

        other.mTexture = nullptr;
        other.mWidth = 0;
        other.mHeight = 0;
    }
    return *this;
}

// ==================== 文件加载 ====================

bool FileTexture::loadFromFile(Renderer& renderer, const std::string& filePath)
{
    destroy();

    SDL_Renderer* rawRenderer = renderer.getRawRenderer();
    if (rawRenderer == nullptr) {
        LOG_ERROR("FileTexture 加载失败: 渲染器尚未创建");
        return false;
    }

    mTexture = IMG_LoadTexture(rawRenderer, filePath.c_str());

    if (mTexture == nullptr) {
        LOG_ERROR("FileTexture 加载失败 (%s): %s", filePath.c_str(), SDL_GetError());
        return false;
    }

    float w = 0.0f, h = 0.0f;
    SDL_GetTextureSize(mTexture, &w, &h);
    mWidth = static_cast<int>(w);
    mHeight = static_cast<int>(h);
    mFilePath = filePath;

    return true;
}

const std::string& FileTexture::getFilePath() const
{
    return mFilePath;
}

// ==================== 实现基类接口 ====================

SDL_Texture* FileTexture::getRawTexture() const
{
    return mTexture;
}

int FileTexture::getWidth() const
{
    return mWidth;
}

int FileTexture::getHeight() const
{
    return mHeight;
}

bool FileTexture::setColorMod(Uint8 r, Uint8 g, Uint8 b)
{
    if (mTexture == nullptr) {
        return false;
    }
    return SDL_SetTextureColorMod(mTexture, r, g, b);
}

bool FileTexture::setAlphaMod(Uint8 alpha)
{
    if (mTexture == nullptr) {
        return false;
    }
    return SDL_SetTextureAlphaMod(mTexture, alpha);
}

Uint8 FileTexture::getAlphaMod() const
{
    if (mTexture == nullptr) {
        return 255;
    }
    Uint8 alpha = 255;
    SDL_GetTextureAlphaMod(mTexture, &alpha);
    return alpha;
}

bool FileTexture::setBlendMode(SDL_BlendMode blendMode)
{
    if (mTexture == nullptr) {
        return false;
    }
    return SDL_SetTextureBlendMode(mTexture, blendMode);
}

bool FileTexture::isValid() const
{
    return mTexture != nullptr;
}

void FileTexture::destroy()
{
    if (mTexture != nullptr) {
        SDL_DestroyTexture(mTexture);
        mTexture = nullptr;
    }
    mWidth = 0;
    mHeight = 0;
    mFilePath.clear();
}
