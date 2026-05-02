/**
 * Font.cpp - 字体与文字渲染类的实现（向后兼容包装）
 *
 * 封装 BaseFont 并保持原有 API 接口。
 */

#include "Font.h"
#include "Renderer.h"
#include "Log.h"
#include "TrueTypeFont.h"
#include "MemoryFont.h"
#include "FontFactory.h"

// ==================== 构造与析构 ====================

Font::Font()
    : mFont(nullptr)
{
}

Font::Font(const std::string& filePath, float fontSize)
    : mFont(nullptr)
{
    load(filePath, fontSize);
}

Font::~Font()
{
    destroy();
}

// ==================== 移动语义 ====================

Font::Font(Font&& other) noexcept
    : mFont(std::move(other.mFont))
{
}

Font& Font::operator=(Font&& other) noexcept
{
    if (this != &other) {
        destroy();
        mFont = std::move(other.mFont);
    }
    return *this;
}

// ==================== 字体加载与管理 ====================

bool Font::load(const std::string& filePath, float fontSize)
{
    destroy();

    mFont = FontFactory::createFromFile(filePath, fontSize);
    if (!mFont || !mFont->isValid()) {
        LOG_ERROR("load 失败 (%s): %s", filePath.c_str(), SDL_GetError());
        mFont.reset();
        return false;
    }

    return true;
}

bool Font::loadFromMemory(const void* data, size_t dataSize, float fontSize)
{
    destroy();

    if (data == nullptr || dataSize == 0) {
        LOG_ERROR("loadFromMemory 失败: 数据为空");
        return false;
    }

    mFont = FontFactory::createFromMemory(data, dataSize, fontSize);
    if (!mFont || !mFont->isValid()) {
        LOG_ERROR("loadFromMemory 加载失败: %s", SDL_GetError());
        mFont.reset();
        return false;
    }

    return true;
}

void Font::destroy()
{
    mFont.reset();
}

bool Font::isValid() const
{
    return mFont != nullptr && mFont->isValid();
}

// ==================== 字体属性 ====================

BaseFont* Font::getBaseFont() const
{
    return mFont.get();
}

TTF_Font* Font::getRawFont() const
{
    return mFont ? mFont->getRawFont() : nullptr;
}

bool Font::setFontSize(float fontSize)
{
    if (!mFont) {
        return false;
    }
    return mFont->setFontSize(fontSize);
}

float Font::getFontSize() const
{
    return mFont ? mFont->getFontSize() : 0.0f;
}

void Font::setStyle(TTF_FontStyleFlags style)
{
    if (mFont) {
        mFont->setStyle(style);
    }
}

TTF_FontStyleFlags Font::getStyle() const
{
    return mFont ? mFont->getStyle() : TTF_STYLE_NORMAL;
}

// ==================== 文字渲染 ====================

SDL_Texture* Font::renderText(Renderer& renderer, const std::string& text,
                              Uint8 r, Uint8 g, Uint8 b,
                              RenderQuality quality)
{
    if (!mFont) {
        LOG_ERROR("renderText 失败: 字体尚未加载");
        return nullptr;
    }

    if (text.empty()) {
        return nullptr;
    }

    int qualityInt = 0;
    switch (quality) {
        case RenderQuality::Solid: qualityInt = 0; break;
        case RenderQuality::Shaded: qualityInt = 1; break;
        case RenderQuality::Blended: qualityInt = 2; break;
    }

    SDL_Texture* texture = mFont->renderText(renderer, text, r, g, b, qualityInt);

    if (texture != nullptr) {
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    }

    return texture;
}

SDL_Texture* Font::renderTextShaded(Renderer& renderer, const std::string& text,
                                    Uint8 fgR, Uint8 fgG, Uint8 fgB,
                                    Uint8 bgR, Uint8 bgG, Uint8 bgB)
{
    // 使用 Blended 模式和自定义纹理绘制来模拟 Shaded 带背景色效果
    if (!mFont || text.empty()) {
        return nullptr;
    }

    SDL_Texture* textTexture = mFont->renderText(renderer, text, fgR, fgG, fgB, 2);
    if (textTexture == nullptr) {
        return nullptr;
    }

    // 获取文字尺寸
    int textW, textH;
    mFont->measureText(text, textW, textH);

    // 创建背景纹理
    SDL_Renderer* rawRenderer = renderer.getRawRenderer();
    SDL_Texture* bgTexture = SDL_CreateTexture(rawRenderer, SDL_PIXELFORMAT_RGBA8888,
                                                SDL_TEXTUREACCESS_TARGET, textW, textH);
    if (bgTexture == nullptr) {
        SDL_DestroyTexture(textTexture);
        return nullptr;
    }

    // 绘制背景色
    SDL_SetRenderTarget(rawRenderer, bgTexture);
    SDL_SetRenderDrawColor(rawRenderer, bgR, bgG, bgB, 255);
    SDL_RenderClear(rawRenderer);

    // 绘制文字
    SDL_FRect textRect = { 0, 0, (float)textW, (float)textH };
    SDL_RenderTexture(rawRenderer, textTexture, nullptr, &textRect);

    // 恢复渲染目标
    SDL_SetRenderTarget(rawRenderer, nullptr);

    // 销毁临时纹理
    SDL_DestroyTexture(textTexture);

    return bgTexture;
}

SDL_Texture* Font::renderTextWrapped(Renderer& renderer, const std::string& text,
                                     Uint8 r, Uint8 g, Uint8 b,
                                     int wrapWidth,
                                     RenderQuality quality)
{
    if (!mFont || text.empty()) {
        return nullptr;
    }

    // 将文本按 wrapWidth 手动拆分，并逐行渲染
    // 这里的实现比较简单，更完整的实现需要处理文字换行
    // 暂时先使用单行渲染，后面可以扩展
    int qualityInt = 0;
    switch (quality) {
        case RenderQuality::Solid: qualityInt = 0; break;
        case RenderQuality::Shaded: qualityInt = 1; break;
        case RenderQuality::Blended: qualityInt = 2; break;
    }

    SDL_Texture* texture = mFont->renderText(renderer, text, r, g, b, qualityInt);

    if (texture != nullptr) {
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    }

    return texture;
}

bool Font::measureText(const std::string& text, int& width, int& height)
{
    if (!mFont) {
        width = 0;
        height = 0;
        return false;
    }
    return mFont->measureText(text, width, height);
}

// ==================== 工厂方法 ====================

Font Font::createFromFileFactory(const std::string& filePath, float fontSize)
{
    Font font;
    font.load(filePath, fontSize);
    return font;
}
