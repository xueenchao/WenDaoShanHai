/**
 * TrueTypeFont.cpp - TrueType/OpenType 字体类实现
 */

#include "TrueTypeFont.h"
#include "Renderer.h"
#include "Log.h"
#include <SDL3_ttf/SDL_ttf.h>

// ==================== 构造与析构 ====================

TrueTypeFont::TrueTypeFont()
    : mFont(nullptr)
    , mFontSize(0.0f)
{
}

TrueTypeFont::~TrueTypeFont()
{
    destroy();
}

// ==================== 移动语义 ====================

TrueTypeFont::TrueTypeFont(TrueTypeFont&& other) noexcept
    : mFont(other.mFont)
    , mFontSize(other.mFontSize)
    , mFilePath(std::move(other.mFilePath))
{
    other.mFont = nullptr;
    other.mFontSize = 0.0f;
}

TrueTypeFont& TrueTypeFont::operator=(TrueTypeFont&& other) noexcept
{
    if (this != &other) {
        destroy();

        mFont = other.mFont;
        mFontSize = other.mFontSize;
        mFilePath = std::move(other.mFilePath);

        other.mFont = nullptr;
        other.mFontSize = 0.0f;
    }
    return *this;
}

// ==================== 文件加载 ====================

bool TrueTypeFont::loadFromFile(const std::string& filePath, float fontSize)
{
    destroy();

    mFont = TTF_OpenFont(filePath.c_str(), fontSize);
    if (mFont == nullptr) {
        LOG_ERROR("TrueTypeFont 加载失败 (%s): %s", filePath.c_str(), SDL_GetError());
        return false;
    }

    mFontSize = fontSize;
    mFilePath = filePath;

    LOG_DEBUG("TrueTypeFont 加载成功: %s", filePath.c_str());
    return true;
}

const std::string& TrueTypeFont::getFilePath() const
{
    return mFilePath;
}

// ==================== 实现基类接口 ====================

TTF_Font* TrueTypeFont::getRawFont() const
{
    return mFont;
}

float TrueTypeFont::getFontSize() const
{
    return mFontSize;
}

bool TrueTypeFont::setFontSize(float fontSize)
{
    if (mFont == nullptr) {
        return false;
    }

    // 重新加载字体以改变大小
    std::string path = mFilePath;
    float oldSize = mFontSize;
    destroy();

    if (loadFromFile(path, fontSize)) {
        return true;
    }

    // 加载失败，尝试恢复旧状态
    loadFromFile(path, oldSize);
    return false;
}

void TrueTypeFont::setStyle(TTF_FontStyleFlags style)
{
    if (mFont != nullptr) {
        TTF_SetFontStyle(mFont, style);
    }
}

TTF_FontStyleFlags TrueTypeFont::getStyle() const
{
    if (mFont != nullptr) {
        return TTF_GetFontStyle(mFont);
    }
    return TTF_STYLE_NORMAL;
}

bool TrueTypeFont::isValid() const
{
    return mFont != nullptr;
}

void TrueTypeFont::destroy()
{
    if (mFont != nullptr) {
        TTF_CloseFont(mFont);
        mFont = nullptr;
    }
    mFontSize = 0.0f;
    mFilePath.clear();
}

SDL_Texture* TrueTypeFont::renderText(Renderer& renderer, const std::string& text,
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
        case 0: // Solid (快速但边缘锯齿)
            surface = TTF_RenderText_Solid(mFont, text.c_str(), textLength, color);
            break;
        case 1: // Shaded (带背景的抗锯齿)
        {
            SDL_Color bgColor = { 0, 0, 0, 0 };
            surface = TTF_RenderText_Shaded(mFont, text.c_str(), textLength, color, bgColor);
            break;
        }
        case 2: // Blended (最佳透明抗锯齿)
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

bool TrueTypeFont::measureText(const std::string& text, int& width, int& height)
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
