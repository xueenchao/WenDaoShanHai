/**
 * BaseTexture.h - 纹理抽象基类
 *
 * 定义所有纹理类的统一接口，作为工厂模式的产品接口。
 * 封装 SDL3 纹理的基本操作，提供统一的访问方式。
 */

#ifndef BASETEXTURE_H
#define BASETEXTURE_H

#include <SDL3/SDL.h>
#include <string>

// 前向声明
class Renderer;

class BaseTexture {
public:
    // ==================== 构造与析构 ====================
    BaseTexture() = default;
    virtual ~BaseTexture() = default;

    // 禁用拷贝操作
    BaseTexture(const BaseTexture&) = delete;
    BaseTexture& operator=(const BaseTexture&) = delete;

    // 支持移动语义
    BaseTexture(BaseTexture&& other) noexcept = default;
    BaseTexture& operator=(BaseTexture&& other) noexcept = default;

    // ==================== 纹理属性 ====================

    /**
     * 获取 SDL 原生纹理指针
     * @return SDL_Texture 指针
     */
    virtual SDL_Texture* getRawTexture() const = 0;

    /**
     * 获取纹理宽度
     * @return 纹理宽度（像素）
     */
    virtual int getWidth() const = 0;

    /**
     * 获取纹理高度
     * @return 纹理高度（像素）
     */
    virtual int getHeight() const = 0;

    // ==================== 纹理效果 ====================

    /**
     * 设置纹理的颜色调制（整体着色）
     * @param r 红色调制值 (0-255)
     * @param g 绿色调制值 (0-255)
     * @param b 蓝色调制值 (0-255)
     * @return 成功返回 true
     */
    virtual bool setColorMod(Uint8 r, Uint8 g, Uint8 b) = 0;

    /**
     * 设置纹理的透明度
     * @param alpha 透明度 (0=完全透明, 255=完全不透明)
     * @return 成功返回 true
     */
    virtual bool setAlphaMod(Uint8 alpha) = 0;

    /**
     * 获取纹理的透明度
     * @return 当前透明度值 (0-255)
     */
    virtual Uint8 getAlphaMod() const = 0;

    /**
     * 设置纹理的混合模式
     * @param blendMode 混合模式
     * @return 成功返回 true
     */
    virtual bool setBlendMode(SDL_BlendMode blendMode) = 0;

    // ==================== 纹理状态 ====================

    /**
     * 判断纹理是否有效
     * @return 有效返回 true
     */
    virtual bool isValid() const = 0;

    /**
     * 销毁纹理，释放资源
     */
    virtual void destroy() = 0;
};

#endif // BASETEXTURE_H
