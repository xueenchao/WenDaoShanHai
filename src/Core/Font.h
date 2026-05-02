/**
 * Font.h - 字体与文字渲染类（向后兼容包装）
 *
 * 为保持向后兼容性，提供与旧版接口一致的字体类。
 * 内部封装 BaseFont 的实现，简化现有代码的迁移。
 */

#ifndef FONT_H
#define FONT_H

#include "BaseFont.h"
#include "FontFactory.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>

class Renderer;

class Font {
public:
    // ==================== 文字渲染质量 ====================

    enum class RenderQuality {
        Solid,    // 快速渲染，无抗锯齿
        Shaded,   // 带背景色的抗锯齿
        Blended   // 透明抗锯齿
    };

    // ==================== 构造与析构 ====================

    Font();
    Font(const std::string& filePath, float fontSize);
    ~Font();

    Font(const Font&) = delete;
    Font& operator=(const Font&) = delete;

    Font(Font&& other) noexcept;
    Font& operator=(Font&& other) noexcept;

    // ==================== 字体加载与管理 ====================

    bool load(const std::string& filePath, float fontSize);
    bool loadFromMemory(const void* data, size_t dataSize, float fontSize);
    void destroy();
    bool isValid() const;

    // ==================== 字体属性 ====================

    BaseFont* getBaseFont() const;
    TTF_Font* getRawFont() const;
    bool setFontSize(float fontSize);
    float getFontSize() const;
    void setStyle(TTF_FontStyleFlags style);
    TTF_FontStyleFlags getStyle() const;

    // ==================== 文字渲染 ====================

    SDL_Texture* renderText(Renderer& renderer, const std::string& text,
                          Uint8 r, Uint8 g, Uint8 b,
                          RenderQuality quality = RenderQuality::Blended);
    SDL_Texture* renderTextShaded(Renderer& renderer, const std::string& text,
                                 Uint8 r, Uint8 g, Uint8 b,
                                 Uint8 br, Uint8 bg, Uint8 bb);
    SDL_Texture* renderTextWrapped(Renderer& renderer, const std::string& text,
                                  Uint8 r, Uint8 g, Uint8 b,
                                  int wrapWidth,
                                  RenderQuality quality = RenderQuality::Blended);
    bool measureText(const std::string& text, int& width, int& height);

    // ==================== 工厂方法 ====================

    static Font createFromFileFactory(const std::string& filePath, float fontSize);

private:
    std::unique_ptr<BaseFont> mFont;
};

#endif // FONT_H
