/**
 * SurfaceTexture.h - 从 SDL_Surface 转换的纹理类
 *
 * 工厂模式的具体产品类，负责从 SDL_Surface 转换为纹理。
 * 适用于程序化生成图像后转换为纹理的场景。
 */

#ifndef SURFACETEXTURE_H
#define SURFACETEXTURE_H

#include "BaseTexture.h"
#include <SDL3/SDL.h>

class SurfaceTexture : public BaseTexture {
public:
    // ==================== 构造与析构 ====================
    SurfaceTexture();
    ~SurfaceTexture() override;

    // 禁用拷贝
    SurfaceTexture(const SurfaceTexture&) = delete;
    SurfaceTexture& operator=(const SurfaceTexture&) = delete;

    // 支持移动语义
    SurfaceTexture(SurfaceTexture&& other) noexcept;
    SurfaceTexture& operator=(SurfaceTexture&& other) noexcept;

    // ==================== 从 Surface 转换 ====================

    /**
     * 从 SDL_Surface 创建纹理
     * @param renderer 渲染器对象引用
     * @param surface SDL_Surface 指针，创建后此表面可由调用者自行释放
     * @return 创建成功返回 true
     */
    bool createFromSurface(Renderer& renderer, SDL_Surface* surface);

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
};

#endif // SURFACETEXTURE_H
