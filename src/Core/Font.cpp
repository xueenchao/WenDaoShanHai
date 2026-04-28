/**
 * Font.cpp - 字体与文字渲染类的实现
 *
 * 封装 SDL3_ttf 的字体加载与文字渲染。
 */

#include "Font.h"
#include "Renderer.h"
#include "Log.h"

// ==================== 构造与析构 ====================

Font::Font()
    : mFont(nullptr)
    , mFontSize(0.0f)
{
}

Font::Font(const std::string& filePath, float fontSize)
    : mFont(nullptr)
    , mFontSize(0.0f)
{
    load(filePath, fontSize);
}

Font::~Font()
{
    destroy();
}

// ==================== 移动语义 ====================

Font::Font(Font&& other) noexcept
    : mFont(other.mFont)
    , mFontSize(other.mFontSize)
{
    other.mFont = nullptr;
    other.mFontSize = 0.0f;
}

Font& Font::operator=(Font&& other) noexcept
{
    if (this != &other) {
        destroy();
        mFont = other.mFont;
        mFontSize = other.mFontSize;
        other.mFont = nullptr;
        other.mFontSize = 0.0f;
    }
    return *this;
}

// ==================== 字体加载与管理 ====================

bool Font::load(const std::string& filePath, float fontSize)
{
    // 先释放已有字体
    destroy();

    // 调用 SDL3_ttf API 加载字体文件
    // ptsize 参数是浮点数（SDL3_ttf 支持小数字号）
    mFont = TTF_OpenFont(filePath.c_str(), fontSize);

    if (mFont == nullptr) {
        LOG_ERROR("load 失败 (%s): %s", filePath.c_str(), SDL_GetError());
        return false;
    }

    mFontSize = fontSize;
    return true;
}

bool Font::loadFromMemory(const void* data, size_t dataSize, float fontSize)
{
    destroy();

    if (data == nullptr || dataSize == 0) {
        LOG_ERROR("loadFromMemory 失败: 数据为空");
        return false;
    }

    // 从内存数据创建 IOStream
    SDL_IOStream* io = SDL_IOFromConstMem(data, dataSize);
    if (io == nullptr) {
        LOG_ERROR("loadFromMemory 创建IOStream失败: %s", SDL_GetError());
        return false;
    }

    // 通过 IOStream 加载字体
    // closeio = true 表示加载完成后自动关闭 IOStream
    mFont = TTF_OpenFontIO(io, true, fontSize);

    if (mFont == nullptr) {
        LOG_ERROR("loadFromMemory 加载失败: %s", SDL_GetError());
        return false;
    }

    mFontSize = fontSize;
    return true;
}

void Font::destroy()
{
    if (mFont != nullptr) {
        TTF_CloseFont(mFont);
        mFont = nullptr;
    }
    mFontSize = 0.0f;
}

bool Font::isValid() const
{
    return mFont != nullptr;
}

// ==================== 字体属性 ====================

TTF_Font* Font::getRawFont() const
{
    return mFont;
}

bool Font::setFontSize(float fontSize)
{
    if (mFont == nullptr) {
        return false;
    }
    // SDL3_ttf 支持动态修改字体大小，无需重新加载字体文件
    if (!TTF_SetFontSize(mFont, fontSize)) {
        LOG_ERROR("setFontSize 失败: %s", SDL_GetError());
        return false;
    }
    mFontSize = fontSize;
    return true;
}

float Font::getFontSize() const
{
    return mFontSize;
}

void Font::setStyle(TTF_FontStyleFlags style)
{
    if (mFont == nullptr) {
        return;
    }
    TTF_SetFontStyle(mFont, style);
}

TTF_FontStyleFlags Font::getStyle() const
{
    if (mFont == nullptr) {
        return TTF_STYLE_NORMAL;
    }
    return TTF_GetFontStyle(mFont);
}

// ==================== 文字渲染 ====================

SDL_Texture* Font::renderText(Renderer& renderer, const std::string& text,
                              Uint8 r, Uint8 g, Uint8 b,
                              RenderQuality quality)
{
    if (mFont == nullptr) {
        LOG_ERROR("renderText 失败: 字体尚未加载");
        return nullptr;
    }

    // 文字为空时不渲染
    if (text.empty()) {
        return nullptr;
    }

    SDL_Surface* surface = nullptr;

    // 根据渲染质量选择不同的渲染方法
    switch (quality) {
    case RenderQuality::Solid:
        // Solid 模式：速度快，无抗锯齿，适合调试信息
        surface = TTF_RenderText_Solid(mFont, text.c_str(), text.length(), {r, g, b, 255});
        break;

    case RenderQuality::Shaded:
        // Shaded 模式需要背景色，这里默认使用黑色背景
        // 如果需要自定义背景色，请使用 renderTextShaded 方法
        surface = TTF_RenderText_Shaded(mFont, text.c_str(), text.length(),
                                        {r, g, b, 255}, {0, 0, 0, 255});
        break;

    case RenderQuality::Blended:
        // Blended 模式：透明抗锯齿，效果最好，适合游戏标题和重要文字
        surface = TTF_RenderText_Blended(mFont, text.c_str(), text.length(), {r, g, b, 255});
        break;
    }

    if (surface == nullptr) {
        LOG_ERROR("renderText 渲染失败: %s", SDL_GetError());
        return nullptr;
    }

    // 将 Surface 转换为 Texture
    SDL_Renderer* rawRenderer = renderer.getRawRenderer();
    SDL_Texture* texture = SDL_CreateTextureFromSurface(rawRenderer, surface);

    // Surface 不再需要，立即释放
    SDL_DestroySurface(surface);

    if (texture == nullptr) {
        LOG_ERROR("renderText 创建纹理失败: %s", SDL_GetError());
        return nullptr;
    }

    // 启用透明混合，使文字背景透明
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    return texture;
}

SDL_Texture* Font::renderTextShaded(Renderer& renderer, const std::string& text,
                                    Uint8 fgR, Uint8 fgG, Uint8 fgB,
                                    Uint8 bgR, Uint8 bgG, Uint8 bgB)
{
    if (mFont == nullptr || text.empty()) {
        return nullptr;
    }

    // Shaded 模式：带背景色的抗锯齿渲染
    SDL_Surface* surface = TTF_RenderText_Shaded(
        mFont, text.c_str(), text.length(),
        {fgR, fgG, fgB, 255}, {bgR, bgG, bgB, 255});

    if (surface == nullptr) {
        LOG_ERROR("renderTextShaded 渲染失败: %s", SDL_GetError());
        return nullptr;
    }

    SDL_Renderer* rawRenderer = renderer.getRawRenderer();
    SDL_Texture* texture = SDL_CreateTextureFromSurface(rawRenderer, surface);
    SDL_DestroySurface(surface);

    if (texture == nullptr) {
        LOG_ERROR("renderTextShaded 创建纹理失败: %s", SDL_GetError());
        return nullptr;
    }

    return texture;
}

SDL_Texture* Font::renderTextWrapped(Renderer& renderer, const std::string& text,
                                     Uint8 r, Uint8 g, Uint8 b,
                                     int wrapWidth,
                                     RenderQuality quality)
{
    if (mFont == nullptr || text.empty()) {
        return nullptr;
    }

    SDL_Surface* surface = nullptr;

    switch (quality) {
    case RenderQuality::Solid:
        // 自动换行的 Solid 渲染
        surface = TTF_RenderText_Solid_Wrapped(
            mFont, text.c_str(), text.length(), {r, g, b, 255}, wrapWidth);
        break;

    case RenderQuality::Shaded:
        // 自动换行的 Shaded 渲染（黑色背景）
        surface = TTF_RenderText_Shaded_Wrapped(
            mFont, text.c_str(), text.length(),
            {r, g, b, 255}, {0, 0, 0, 255}, wrapWidth);
        break;

    case RenderQuality::Blended:
        // 自动换行的 Blended 渲染
        surface = TTF_RenderText_Blended_Wrapped(
            mFont, text.c_str(), text.length(), {r, g, b, 255}, wrapWidth);
        break;
    }

    if (surface == nullptr) {
        LOG_ERROR("renderTextWrapped 渲染失败: %s", SDL_GetError());
        return nullptr;
    }

    SDL_Renderer* rawRenderer = renderer.getRawRenderer();
    SDL_Texture* texture = SDL_CreateTextureFromSurface(rawRenderer, surface);
    SDL_DestroySurface(surface);

    if (texture == nullptr) {
        LOG_ERROR("renderTextWrapped 创建纹理失败: %s", SDL_GetError());
        return nullptr;
    }

    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    return texture;
}

bool Font::measureText(const std::string& text, int& width, int& height)
{
    if (mFont == nullptr) {
        width = 0;
        height = 0;
        return false;
    }

    // TTF_GetStringSize 只计算文字尺寸，不实际渲染，非常高效
    return TTF_GetStringSize(mFont, text.c_str(), text.length(), &width, &height);
}
