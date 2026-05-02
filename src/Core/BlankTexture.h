/**
 * BlankTexture.h - 空白纹理类
 *
 * 工厂模式的具体产品类，用于创建空白纹理。
 * 空白纹理常用于渲染目标或程序化生成的图像。
 */

#ifndef BLANKTEXTURE_H
#define BLANKTEXTURE_H

#include "BaseTexture.h"

class BlankTexture : public BaseTexture {
public:
    // ==================== 构造与析构 ====================
    BlankTexture();
    ~BlankTexture() override;

    // 禁用拷贝
    BlankTexture(const BlankTexture&) = delete;
    BlankTexture& operator=(const BlankTexture&) = delete;

    // 支持移动语义
    BlankTexture(BlankTexture&& other) noexcept;
    BlankTexture& operator=(BlankTexture&& other) noexcept;

    // ==================== 创建空白纹理 ====================

    /**
     * 创建一个空白的纹理
     * @param renderer 渲染器对象引用
     * @param width 纹理宽度
     * @param height 纹理高度
     * @param access 访问模式
     * @return 创建成功返回 true
     */
    bool createBlank(Renderer& renderer, int width, int height,
                     SDL_TextureAccess access = SDL_TEXTUREACCESS_TARGET);

    /**
     * 检查是否为渲染目标
     * @return 是渲染目标返回 true
     */
    bool isRenderTarget() const;

    // ==================== 实现基类接口 ====================
    SDL_Texture* getRawTexture() const override;
    int getWidth() const override;
    int getHeight() const override;
    bool setColorMod(Uint8 r, Uint8 g, Uint8 b) override;
    bool setAlphaMod(Uint8 alpha) override;
    Uint8 getAlphaMod() const override;
    bool setBlendMode(SDL_BlendMode blendMode) override;
    bool isValid() const override;
    void destroy() override;

private:
    SDL_Texture* mTexture;
    int mWidth;
    int mHeight;
    SDL_TextureAccess mAccess;
};

#endif // BLANKTEXTURE_H
