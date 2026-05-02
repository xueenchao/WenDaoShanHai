/**
 * Texture.h - 纹理与图片管理类（向后兼容包装）
 *
 * 为保持向后兼容性，提供与旧版接口一致的纹理类。
 * 内部封装 BaseTexture 的实现，简化现有代码的迁移。
 */

#ifndef TEXTURE_H
#define TEXTURE_H

#include "BaseTexture.h"
#include "TextureFactory.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <string>

// 前向声明
class Renderer;

class Texture {
public:
    // ==================== 构造与析构 ====================

    /**
     * 默认构造函数：创建一个空的纹理对象
     */
    Texture();

    /**
     * 析构函数：自动释放纹理资源
     */
    ~Texture();

    // 禁用拷贝
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    // 支持移动语义
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    // ==================== 纹理创建与加载 ====================

    /**
     * 从图片文件加载纹理
     * 支持 BMP、PNG、JPG、WEBP 等常见图片格式
     * @param renderer 渲染器对象引用
     * @param filePath 图片文件路径
     * @return 加载成功返回 true
     */
    bool loadFromFile(Renderer& renderer, const std::string& filePath);

    /**
     * 从内存数据加载纹理
     * 适用于从自定义资源包或网络数据中加载图片
     * @param renderer 渲染器对象引用
     * @param data 图片数据的内存指针
     * @param dataSize 图片数据的大小（字节）
     * @return 加载成功返回 true
     */
    bool loadFromMemory(Renderer& renderer, const void* data, size_t dataSize);

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
     * 从 SDL_Surface 创建纹理
     * @param renderer 渲染器对象引用
     * @param surface SDL_Surface 指针
     * @return 创建成功返回 true
     */
    bool createFromSurface(Renderer& renderer, SDL_Surface* surface);

    /**
     * 销毁纹理，释放资源
     */
    void destroy();

    /**
     * 判断纹理是否有效
     * @return 有效返回 true
     */
    bool isValid() const;

    // ==================== 纹理属性 ====================

    /**
     * 获取 SDL 原生纹理指针
     * @return SDL_Texture 指针
     */
    SDL_Texture* getRawTexture() const;

    /**
     * 获取纹理宽度
     * @return 纹理宽度（像素）
     */
    int getWidth() const;

    /**
     * 获取纹理高度
     * @return 纹理高度（像素）
     */
    int getHeight() const;

    // ==================== 纹理效果 ====================

    /**
     * 设置纹理的颜色调制
     * @param r 红色调制值 (0-255)
     * @param g 绿色调制值 (0-255)
     * @param b 蓝色调制值 (0-255)
     * @return 成功返回 true
     */
    bool setColorMod(Uint8 r, Uint8 g, Uint8 b);

    /**
     * 设置纹理的透明度
     * @param alpha 透明度 (0=完全透明, 255=完全不透明)
     * @return 成功返回 true
     */
    bool setAlphaMod(Uint8 alpha);

    /**
     * 获取纹理的透明度
     * @return 当前透明度值 (0-255)
     */
    Uint8 getAlphaMod() const;

    /**
     * 设置纹理的混合模式
     * @param blendMode 混合模式
     * @return 成功返回 true
     */
    bool setBlendMode(SDL_BlendMode blendMode);

    // ==================== 工厂方法 ====================

    /**
     * 使用工厂模式创建纹理
     * @param renderer 渲染器对象引用
     * @param filePath 图片文件路径
     * @return Texture 对象，加载失败返回无效对象
     */
    static Texture createFromFileFactory(Renderer& renderer, const std::string& filePath);

    /**
     * 获取内部的 BaseTexture 指针（用于与工厂模式交互）
     * @return BaseTexture 指针
     */
    BaseTexture* getBaseTexture() const;

private:
    std::unique_ptr<BaseTexture> mTexture;  // 使用智能指针管理纹理
    int mWidth;                             // 宽度缓存
    int mHeight;                            // 高度缓存
};

#endif // TEXTURE_H
