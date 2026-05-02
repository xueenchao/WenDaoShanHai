/**
 * MemoryFont.h - 从内存加载的字体类
 *
 * 工厂模式的具体产品类，负责从内存加载字体数据。
 * 适用于从自定义资源包或网络数据中加载字体。
 */

#ifndef MEMORYFONT_H
#define MEMORYFONT_H

#include "BaseFont.h"
#include <string>

class MemoryFont : public BaseFont {
public:
    // ==================== 构造与析构 ====================
    MemoryFont();
    ~MemoryFont() override;

    // 禁用拷贝
    MemoryFont(const MemoryFont&) = delete;
    MemoryFont& operator=(const MemoryFont&) = delete;

    // 支持移动语义
    MemoryFont(MemoryFont&& other) noexcept;
    MemoryFont& operator=(MemoryFont&& other) noexcept;

    // ==================== 内存加载 ====================

    /**
     * 从内存加载字体
     * @param data 字体数据的内存指针
     * @param dataSize 数据大小（字节）
     * @param fontSize 字体大小（磅值）
     * @return 加载成功返回 true
     */
    bool loadFromMemory(const void* data, size_t dataSize, float fontSize);

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
    // 保存内存指针的副本，以便重建字体时使用
    void* mCachedData;
    size_t mCachedDataSize;
};

#endif // MEMORYFONT_H
