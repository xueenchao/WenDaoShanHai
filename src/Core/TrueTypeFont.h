/**
 * TrueTypeFont.h - TrueType/OpenType 字体类
 *
 * 工厂模式的具体产品类，负责加载 TTF/OTF 字体文件。
 * 提供字体渲染和测量功能。
 */

#ifndef TRUETYPEFONT_H
#define TRUETYPEFONT_H

#include "BaseFont.h"
#include <string>

class TrueTypeFont : public BaseFont {
public:
    // ==================== 构造与析构 ====================
    TrueTypeFont();
    ~TrueTypeFont() override;

    // 禁用拷贝
    TrueTypeFont(const TrueTypeFont&) = delete;
    TrueTypeFont& operator=(const TrueTypeFont&) = delete;

    // 支持移动语义
    TrueTypeFont(TrueTypeFont&& other) noexcept;
    TrueTypeFont& operator=(TrueTypeFont&& other) noexcept;

    // ==================== 文件加载 ====================

    /**
     * 从文件加载字体
     * @param filePath 文件路径
     * @param fontSize 字体大小（磅值）
     * @return 加载成功返回 true
     */
    bool loadFromFile(const std::string& filePath, float fontSize);

    /**
     * 获取加载的文件路径
     * @return 文件路径
     */
    const std::string& getFilePath() const;

    // ==================== 实现基类接口 ====================
    TTF_Font* getRawFont() const override;
    float getFontSize() const override;
    bool setFontSize(float fontSize) override;
    void setStyle(TTF_FontStyleFlags style) override;
    TTF_FontStyleFlags getStyle() const override;
    bool isValid() const override;
    void destroy() override;
    SDL_Texture* renderText(Renderer& renderer, const std::string& text,
                          Uint8 r, Uint8 g, Uint8 b, int quality = 2) override;
    bool measureText(const std::string& text, int& width, int& height) override;

private:
    TTF_Font* mFont;
    float mFontSize;
    std::string mFilePath;
};

#endif // TRUETYPEFONT_H
