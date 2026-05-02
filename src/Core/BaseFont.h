/**
 * BaseFont.h - 字体抽象基类
 *
 * 定义所有字体类的统一接口，作为工厂模式的产品接口。
 * 封装 SDL3_ttf 的基本操作，提供统一的访问方式。
 */

#ifndef BASEFONT_H
#define BASEFONT_H

#include <SDL3_ttf/SDL_ttf.h>
#include <string>

// 前向声明
class Renderer;

class BaseFont {
public:
    // ==================== 构造与析构 ====================
    BaseFont() = default;
    virtual ~BaseFont() = default;

    // 禁用拷贝
    BaseFont(const BaseFont&) = delete;
    BaseFont& operator=(const BaseFont&) = delete;

    // 支持移动语义
    BaseFont(BaseFont&& other) noexcept = default;
    BaseFont& operator=(BaseFont&& other) noexcept = default;

    // ==================== 字体属性 ====================

    /**
     * 获取 SDL3_ttf 原生字体指针
     * @return TTF_Font 指针
     */
    virtual TTF_Font* getRawFont() const = 0;

    /**
     * 获取当前字体大小
     * @return 字体大小（磅值）
     */
    virtual float getFontSize() const = 0;

    /**
     * 设置字体大小
     * @param fontSize 新的字体大小
     * @return 成功返回 true
     */
    virtual bool setFontSize(float fontSize) = 0;

    /**
     * 设置字体样式
     * @param style 样式标志
     */
    virtual void setStyle(TTF_FontStyleFlags style) = 0;

    /**
     * 获取当前字体样式
     * @return 样式标志
     */
    virtual TTF_FontStyleFlags getStyle() const = 0;

    // ==================== 字体有效性 ====================

    /**
     * 判断字体是否有效
     * @return 有效返回 true
     */
    virtual bool isValid() const = 0;

    /**
     * 销毁字体资源
     */
    virtual void destroy() = 0;

    // ==================== 文字渲染 ====================

    /**
     * 将文字渲染为纹理（最高质量）
     * @param renderer 渲染器对象引用
     * @param text 要渲染的文字
     * @param r,g,b 文字颜色
     * @param quality 渲染质量：0=快速，1=带背景，2=透明
     * @return 渲染成功返回 SDL_Texture 指针，调用者负责销毁
     */
    virtual SDL_Texture* renderText(Renderer& renderer, const std::string& text,
                                  Uint8 r, Uint8 g, Uint8 b, int quality = 2) = 0;

    /**
     * 测量文字的渲染尺寸
     * @param text 要测量的文字
     * @param width 输出：文字宽度（像素）
     * @param height 输出：文字高度（像素）
     * @return 成功返回 true
     */
    virtual bool measureText(const std::string& text, int& width, int& height) = 0;
};

#endif // BASEFONT_H
