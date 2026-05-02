/**
 * MemoryFont.cpp - 从内存加载的字体类实现
 */

#include "MemoryFont.h"
#include "Renderer.h"
#include "Log.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

// ==================== 构造与析构 ====================

MemoryFont::MemoryFont()
    : mFont(nullptr)
    , mFontSize(0.0f)
    , mCachedData(nullptr)
    , mCachedDataSize(0)
{
}

MemoryFont::~MemoryFont()
{
    destroy();
}

// ==================== 移动语义 ====================

MemoryFont::MemoryFont(MemoryFont&& other) noexcept
    : mFont(other.mFont)
    , mFontSize(other.mFontSize)
    , mCachedData(other.mCachedData)
    , mCachedDataSize(other.mCachedDataSize)
{
    other.mFont = nullptr;
    other.mFontSize = 0.0f;
    other.mCachedData = nullptr;
    other.mCachedDataSize = 0;
}

MemoryFont& MemoryFont::operator=(MemoryFont&& other) noexcept
{
    if (this != &other) {
        destroy();

        mFont = other.mFont;
        mFontSize = other.mFontSize;
        mCachedData = other.mCachedData;
        mCachedDataSize = other.mCachedDataSize;

        other.mFont = nullptr;
        other.mFontSize = 0.0f;
        other.mCachedData = nullptr;
        other.mCachedDataSize = 0;
    }
    return *this;
}

// ==================== 内存加载 ====================

bool MemoryFont::loadFromMemory(const void* data, size_t dataSize, float fontSize)
{
    destroy();

    if (data == nullptr || dataSize == 0) {
        LOG_ERROR("MemoryFont 加载失败: 数据为空");
        return false;
    }

    // 复制数据到缓存区
    mCachedData = malloc(dataSize);
    if (mCachedData == nullptr) {
        LOG_ERROR("MemoryFont 加载失败: 内存分配失败");
        return false;
    }
    memcpy(mCachedData, data, dataSize);
    mCachedDataSize = dataSize;

    // 从缓存的内存中加载字体
    SDL_IOStream* io = SDL_IOFromConstMem(mCachedData, mCachedDataSize);
    if (io == nullptr) {
        LOG_ERROR("MemoryFont 创建IOStream失败: %s", SDL_GetError());
        destroy();
        return false;
    }

    mFont = TTF_OpenFontIO(io, 1, fontSize);
    if (mFont == nullptr) {
        LOG_ERROR("MemoryFont 加载失败: %s", SDL_GetError());
        destroy();
        return false;
    }

    mFontSize = fontSize;
    LOG_DEBUG("MemoryFont 加载成功");
    return true;
}

// ==================== 实现基类接口 ====================

TTF_Font* MemoryFont::getRawFont() const
{
    return mFont;
}

float MemoryFont::getFontSize() const
{
    return mFontSize;
}

bool MemoryFont::setFontSize(float fontSize)
{
    if (mFont == nullptr || mCachedData == nullptr || mCachedDataSize == 0) {
        return false;
    }

    // 使用缓存数据重新加载字体
    void* oldData = mCachedData;
    size_t oldSize = mCachedDataSize;
    float oldSizeVal = mFontSize;
    destroy();

    if (loadFromMemory(oldData, oldSize, fontSize)) {
        free(oldData); // loadFromMemory 复制了数据，这里释放旧副本
        return true;
    }

    // 加载失败，尝试恢复旧状态
    loadFromMemory(oldData, oldSize, oldSizeVal);
    free(oldData);
    return false;
}

void MemoryFont::setStyle(TTF_FontStyleFlags style)
{
    if (mFont != nullptr) {
        TTF_SetFontStyle(mFont, style);
    }
}

TTF_FontStyleFlags MemoryFont::getStyle() const
{
    if (mFont != nullptr) {
        return TTF_GetFontStyle(mFont);
    }
    return TTF_STYLE_NORMAL;
}

bool MemoryFont::isValid() const
{
    return mFont != nullptr;
}

void MemoryFont::destroy()
{
    if (mFont != nullptr) {
        TTF_CloseFont(mFont);
        mFont = nullptr;
    }
    if (mCachedData != nullptr) {
        free(mCachedData);
        mCachedData = nullptr;
    }
    mFontSize = 0.0f;
    mCachedDataSize = 0;
}

SDL_Texture* MemoryFont::renderText(Renderer& renderer, const std::string& text,
                                   Uint8 r, Uint8 g, Uint8 b, int quality)
{
    if (mFont == nullptr) {
        LOG_ERROR("renderText 失败: 字体未初始化");
        return nullptr;
    }

    SDL_Color color = { r, g, b, 255 };
    SDL_Surface* surface = nullptr;
    size_t textLength = text.length();

    switch (quality) {
        case 0: // Solid
            surface = TTF_RenderText_Solid(mFont, text.c_str(), textLength, color);
            break;
        case 1: // Shaded
        {
            SDL_Color bgColor = { 0, 0, 0, 0 };
            surface = TTF_RenderText_Shaded(mFont, text.c_str(), textLength, color, bgColor);
            break;
        }
        case 2: // Blended
        default:
            surface = TTF_RenderText_Blended(mFont, text.c_str(), textLength, color);
            break;
    }

    if (surface == nullptr) {
        LOG_ERROR("renderText 失败: %s", SDL_GetError());
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer.getRawRenderer(), surface);
    SDL_DestroySurface(surface);

    if (texture == nullptr) {
        LOG_ERROR("createTextureFromSurface 失败: %s", SDL_GetError());
        return nullptr;
    }

    return texture;
}

bool MemoryFont::measureText(const std::string& text, int& width, int& height)
{
    if (mFont == nullptr) {
        LOG_ERROR("measureText 失败: 字体未初始化");
        return false;
    }

    // 测量文本宽度
    size_t textLength = text.length();
    int measuredWidth = 0;
    size_t measuredLength = 0;

    bool result = TTF_MeasureString(mFont, text.c_str(), textLength, 0, &measuredWidth, &measuredLength);
    width = measuredWidth;

    // 获取字体高度
    height = TTF_GetFontHeight(mFont);

    return result;
}
