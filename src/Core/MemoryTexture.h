/**
 * MemoryTexture.h - 从内存加载的纹理类
 *
 * 工厂模式的具体产品类，负责从内存数据加载图片纹理。
 * 适用于从自定义资源包或网络数据中加载图片的场景。
 */

#ifndef MEMORYTEXTURE_H
#define MEMORYTEXTURE_H

#include "BaseTexture.h"

class MemoryTexture : public BaseTexture {
public:
    // ==================== 构造与析构 ====================
    MemoryTexture();
    ~MemoryTexture() override;

    // 禁用拷贝
    MemoryTexture(const MemoryTexture&) = delete;
    MemoryTexture& operator=(const MemoryTexture&) = delete;

    // 支持移动语义
    MemoryTexture(MemoryTexture&& other) noexcept;
    MemoryTexture& operator=(MemoryTexture&& other) noexcept;

    // ==================== 内存加载 ====================

    /**
     * 从内存数据加载纹理
     * @param renderer 渲染器对象引用
     * @param data 图片数据的内存指针
     * @param dataSize 图片数据的大小（字节）
     * @return 加载成功返回 true
     */
    bool loadFromMemory(Renderer& renderer, const void* data, size_t dataSize);

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

#endif // MEMORYTEXTURE_H
