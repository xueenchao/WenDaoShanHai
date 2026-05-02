/**
 * TextureFactory.h - 纹理工厂类
 *
 * 简单工厂模式实现，负责创建各种类型的纹理对象。
 * 封装纹理的创建过程，提高代码的可扩展性和可维护性。
 *
 * 支持创建的纹理类型：
 *   - FileTexture：从文件加载
 *   - MemoryTexture：从内存加载
 *   - BlankTexture：空白纹理
 *   - SurfaceTexture：从 SDL_Surface 转换
 */

#ifndef TEXTUREFACTORY_H
#define TEXTUREFACTORY_H

#include <memory>
#include <string>
#include <SDL3/SDL.h>

// 前向声明
class Renderer;
class BaseTexture;
class FileTexture;
class MemoryTexture;
class BlankTexture;
class SurfaceTexture;

class TextureFactory {
public:
    // ==================== 工厂方法 ====================

    /**
     * 从文件加载纹理
     * @param renderer 渲染器对象引用
     * @param filePath 图片文件路径
     * @return 纹理对象的智能指针，失败返回 nullptr
     */
    static std::unique_ptr<BaseTexture> createFromFile(Renderer& renderer, const std::string& filePath);

    /**
     * 从内存数据加载纹理
     * @param renderer 渲染器对象引用
     * @param data 图片数据的内存指针
     * @param dataSize 图片数据的大小（字节）
     * @return 纹理对象的智能指针，失败返回 nullptr
     */
    static std::unique_ptr<BaseTexture> createFromMemory(Renderer& renderer, const void* data, size_t dataSize);

    /**
     * 创建空白纹理
     * @param renderer 渲染器对象引用
     * @param width 纹理宽度
     * @param height 纹理高度
     * @param access 访问模式
     * @return 纹理对象的智能指针，失败返回 nullptr
     */
    static std::unique_ptr<BaseTexture> createBlank(Renderer& renderer, int width, int height,
                                                    SDL_TextureAccess access = SDL_TEXTUREACCESS_TARGET);

    /**
     * 从 SDL_Surface 创建纹理
     * @param renderer 渲染器对象引用
     * @param surface SDL_Surface 指针
     * @return 纹理对象的智能指针，失败返回 nullptr
     */
    static std::unique_ptr<BaseTexture> createFromSurface(Renderer& renderer, SDL_Surface* surface);

    // ==================== 类型安全的工厂方法 ====================

    /**
     * 创建 FileTexture 对象并从文件加载
     * @param renderer 渲染器对象引用
     * @param filePath 图片文件路径
     * @return FileTexture 的智能指针，失败返回 nullptr
     */
    static std::unique_ptr<FileTexture> createFileTexture(Renderer& renderer, const std::string& filePath);

    /**
     * 创建 MemoryTexture 对象并从内存加载
     * @param renderer 渲染器对象引用
     * @param data 图片数据的内存指针
     * @param dataSize 图片数据的大小（字节）
     * @return MemoryTexture 的智能指针，失败返回 nullptr
     */
    static std::unique_ptr<MemoryTexture> createMemoryTexture(Renderer& renderer, const void* data, size_t dataSize);

    /**
     * 创建 BlankTexture 对象
     * @param renderer 渲染器对象引用
     * @param width 纹理宽度
     * @param height 纹理高度
     * @param access 访问模式
     * @return BlankTexture 的智能指针，失败返回 nullptr
     */
    static std::unique_ptr<BlankTexture> createBlankTexture(Renderer& renderer, int width, int height,
                                                           SDL_TextureAccess access = SDL_TEXTUREACCESS_TARGET);

    /**
     * 创建 SurfaceTexture 对象并从 Surface 加载
     * @param renderer 渲染器对象引用
     * @param surface SDL_Surface 指针
     * @return SurfaceTexture 的智能指针，失败返回 nullptr
     */
    static std::unique_ptr<SurfaceTexture> createSurfaceTexture(Renderer& renderer, SDL_Surface* surface);

private:
    // 私有构造函数，防止创建工厂实例
    TextureFactory() = delete;
    ~TextureFactory() = delete;
    TextureFactory(const TextureFactory&) = delete;
    TextureFactory& operator=(const TextureFactory&) = delete;
};

#endif // TEXTUREFACTORY_H
