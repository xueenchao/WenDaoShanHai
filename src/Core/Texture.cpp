/**
 * Texture.cpp - 纹理与图片管理类的实现
 *
 * 封装 SDL3 + SDL3_image 的纹理加载与管理。
 */

#include "Texture.h"
#include "Renderer.h"
#include "Log.h"

// ==================== 构造与析构 ====================

Texture::Texture()
    : mTexture(nullptr)
    , mWidth(0)
    , mHeight(0)
{
}

Texture::~Texture()
{
    // 析构时自动释放纹理资源
    destroy();
}

// ==================== 移动语义 ====================

Texture::Texture(Texture&& other) noexcept
    : mTexture(other.mTexture)
    , mWidth(other.mWidth)
    , mHeight(other.mHeight)
{
    // 转移所有权后，将源对象置空，防止双重释放
    other.mTexture = nullptr;
    other.mWidth = 0;
    other.mHeight = 0;
}

Texture& Texture::operator=(Texture&& other) noexcept
{
    if (this != &other) {
        // 先释放自己原有的纹理
        destroy();

        // 转移源对象的资源
        mTexture = other.mTexture;
        mWidth = other.mWidth;
        mHeight = other.mHeight;

        // 将源对象置空
        other.mTexture = nullptr;
        other.mWidth = 0;
        other.mHeight = 0;
    }
    return *this;
}

// ==================== 纹理创建与加载 ====================

bool Texture::loadFromFile(Renderer& renderer, const std::string& filePath)
{
    // 先释放已有的纹理
    destroy();

    // 获取渲染器的原生指针
    SDL_Renderer* rawRenderer = renderer.getRawRenderer();
    if (rawRenderer == nullptr) {
        LOG_ERROR("loadFromFile 失败: 渲染器尚未创建");
        return false;
    }

    // 使用 SDL3_image 从文件加载纹理
    // IMG_LoadTexture 内部完成：加载图片 → 创建 Surface → 转换为 Texture → 销毁 Surface
    // 这比手动操作更高效，且自动处理像素格式转换
    mTexture = IMG_LoadTexture(rawRenderer, filePath.c_str());

    if (mTexture == nullptr) {
        LOG_ERROR("loadFromFile 失败 (%s): %s", filePath.c_str(), SDL_GetError());
        return false;
    }

    // 获取并缓存纹理大小
    float w = 0.0f, h = 0.0f;
    SDL_GetTextureSize(mTexture, &w, &h);
    mWidth = static_cast<int>(w);
    mHeight = static_cast<int>(h);

    return true;
}

bool Texture::loadFromMemory(Renderer& renderer, const void* data, size_t dataSize)
{
    destroy();

    SDL_Renderer* rawRenderer = renderer.getRawRenderer();
    if (rawRenderer == nullptr) {
        LOG_ERROR("loadFromMemory 失败: 渲染器尚未创建");
        return false;
    }

    if (data == nullptr || dataSize == 0) {
        LOG_ERROR("loadFromMemory 失败: 数据为空");
        return false;
    }

    // 从内存数据创建 SDL_IOStream（类似文件流，但数据在内存中）
    SDL_IOStream* io = SDL_IOFromConstMem(data, dataSize);
    if (io == nullptr) {
        LOG_ERROR("loadFromMemory 创建IOStream失败: %s", SDL_GetError());
        return false;
    }

    // 通过 IOStream 加载纹理
    // closeio = true 表示加载完成后自动关闭 IOStream
    mTexture = IMG_LoadTexture_IO(rawRenderer, io, true);

    if (mTexture == nullptr) {
        LOG_ERROR("loadFromMemory 加载失败: %s", SDL_GetError());
        return false;
    }

    // 获取并缓存纹理大小
    float w = 0.0f, h = 0.0f;
    SDL_GetTextureSize(mTexture, &w, &h);
    mWidth = static_cast<int>(w);
    mHeight = static_cast<int>(h);

    return true;
}

bool Texture::createBlank(Renderer& renderer, int width, int height,
                          SDL_TextureAccess access)
{
    destroy();

    SDL_Renderer* rawRenderer = renderer.getRawRenderer();
    if (rawRenderer == nullptr) {
        LOG_ERROR("createBlank 失败: 渲染器尚未创建");
        return false;
    }

    // 创建空白纹理
    // SDL_PIXELFORMAT_RGBA8888 是最常用的像素格式，支持透明度
    mTexture = SDL_CreateTexture(rawRenderer, SDL_PIXELFORMAT_RGBA8888,
                                 access, width, height);

    if (mTexture == nullptr) {
        LOG_ERROR("createBlank 失败: %s", SDL_GetError());
        return false;
    }

    mWidth = width;
    mHeight = height;

    // 如果是渲染目标纹理，默认启用混合模式
    if (access == SDL_TEXTUREACCESS_TARGET) {
        SDL_SetTextureBlendMode(mTexture, SDL_BLENDMODE_BLEND);
    }

    return true;
}

bool Texture::createFromSurface(Renderer& renderer, SDL_Surface* surface)
{
    destroy();

    SDL_Renderer* rawRenderer = renderer.getRawRenderer();
    if (rawRenderer == nullptr || surface == nullptr) {
        LOG_ERROR("createFromSurface 失败: 渲染器或Surface无效");
        return false;
    }

    // 从 Surface 创建纹理
    // 内部会自动转换像素格式以匹配渲染器
    mTexture = SDL_CreateTextureFromSurface(rawRenderer, surface);

    if (mTexture == nullptr) {
        LOG_ERROR("createFromSurface 失败: %s", SDL_GetError());
        return false;
    }

    // 从 Surface 获取大小
    mWidth = surface->w;
    mHeight = surface->h;

    return true;
}

void Texture::destroy()
{
    if (mTexture != nullptr) {
        SDL_DestroyTexture(mTexture);
        mTexture = nullptr;
    }
    mWidth = 0;
    mHeight = 0;
}

bool Texture::isValid() const
{
    return mTexture != nullptr;
}

// ==================== 纹理属性 ====================

SDL_Texture* Texture::getRawTexture() const
{
    return mTexture;
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
    if (mTexture == nullptr) {
        return false;
    }
    return SDL_SetTextureColorMod(mTexture, r, g, b);
}

bool Texture::setAlphaMod(Uint8 alpha)
{
    if (mTexture == nullptr) {
        return false;
    }
    return SDL_SetTextureAlphaMod(mTexture, alpha);
}

Uint8 Texture::getAlphaMod() const
{
    if (mTexture == nullptr) {
        return 255;
    }
    Uint8 alpha = 255;
    SDL_GetTextureAlphaMod(mTexture, &alpha);
    return alpha;
}

bool Texture::setBlendMode(SDL_BlendMode blendMode)
{
    if (mTexture == nullptr) {
        return false;
    }
    return SDL_SetTextureBlendMode(mTexture, blendMode);
}
