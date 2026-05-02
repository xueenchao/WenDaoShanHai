/**
 * Texture.cpp - 纹理与图片管理类的实现（向后兼容包装）
 *
 * 封装 SDL3 + SDL3_image 的纹理加载与管理，使用工厂模式。
 */

#include "Texture.h"
#include "Renderer.h"
#include "Log.h"
#include "TextureFactory.h"

// ==================== 构造与析构 ====================

Texture::Texture()
    : mTexture(nullptr)
    , mWidth(0)
    , mHeight(0)
{
}

Texture::~Texture()
{
    destroy();
}

// ==================== 移动语义 ====================

Texture::Texture(Texture&& other) noexcept
    : mTexture(std::move(other.mTexture))
    , mWidth(other.mWidth)
    , mHeight(other.mHeight)
{
    other.mWidth = 0;
    other.mHeight = 0;
}

Texture& Texture::operator=(Texture&& other) noexcept
{
    if (this != &other) {
        destroy();
        mTexture = std::move(other.mTexture);
        mWidth = other.mWidth;
        mHeight = other.mHeight;
        other.mWidth = 0;
        other.mHeight = 0;
    }
    return *this;
}

// ==================== 纹理创建与加载 ====================

bool Texture::loadFromFile(Renderer& renderer, const std::string& filePath)
{
    destroy();
    auto texture = TextureFactory::createFromFile(renderer, filePath);
    if (texture) {
        mWidth = texture->getWidth();
        mHeight = texture->getHeight();
        mTexture = std::move(texture);
        return true;
    }
    return false;
}

bool Texture::loadFromMemory(Renderer& renderer, const void* data, size_t dataSize)
{
    destroy();
    auto texture = TextureFactory::createFromMemory(renderer, data, dataSize);
    if (texture) {
        mWidth = texture->getWidth();
        mHeight = texture->getHeight();
        mTexture = std::move(texture);
        return true;
    }
    return false;
}

bool Texture::createBlank(Renderer& renderer, int width, int height,
                          SDL_TextureAccess access)
{
    destroy();
    auto texture = TextureFactory::createBlank(renderer, width, height, access);
    if (texture) {
        mWidth = texture->getWidth();
        mHeight = texture->getHeight();
        mTexture = std::move(texture);
        return true;
    }
    return false;
}

bool Texture::createFromSurface(Renderer& renderer, SDL_Surface* surface)
{
    destroy();
    auto texture = TextureFactory::createFromSurface(renderer, surface);
    if (texture) {
        mWidth = texture->getWidth();
        mHeight = texture->getHeight();
        mTexture = std::move(texture);
        return true;
    }
    return false;
}

void Texture::destroy()
{
    mTexture.reset();
    mWidth = 0;
    mHeight = 0;
}

bool Texture::isValid() const
{
    return mTexture != nullptr && mTexture->isValid();
}

// ==================== 纹理属性 ====================

SDL_Texture* Texture::getRawTexture() const
{
    return mTexture ? mTexture->getRawTexture() : nullptr;
}

int Texture::getWidth() const
{
    return mWidth;
}

int Texture::getHeight() const
{
    return mHeight;
}

// ==================== 纹理效果 ====================

bool Texture::setColorMod(Uint8 r, Uint8 g, Uint8 b)
{
    return mTexture ? mTexture->setColorMod(r, g, b) : false;
}

bool Texture::setAlphaMod(Uint8 alpha)
{
    return mTexture ? mTexture->setAlphaMod(alpha) : false;
}

Uint8 Texture::getAlphaMod() const
{
    return mTexture ? mTexture->getAlphaMod() : 255;
}

bool Texture::setBlendMode(SDL_BlendMode blendMode)
{
    return mTexture ? mTexture->setBlendMode(blendMode) : false;
}

// ==================== 工厂方法 ====================

Texture Texture::createFromFileFactory(Renderer& renderer, const std::string& filePath)
{
    Texture texture;
    texture.loadFromFile(renderer, filePath);
    return texture;
}

BaseTexture* Texture::getBaseTexture() const
{
    return mTexture.get();
}

