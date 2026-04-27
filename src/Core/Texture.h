/**
 * Texture.h - 纹理与图片管理类
 *
 * 封装 SDL3 + SDL3_image 的纹理加载、创建与管理功能。
 * 提供从文件加载图片、创建空白纹理、设置纹理属性等操作。
 */

#ifndef TEXTURE_H
#define TEXTURE_H

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
     * 需要后续调用 loadFromFile() 或 createBlank() 来初始化
     */
    Texture();

    /**
     * 析构函数：自动释放纹理资源
     */
    ~Texture();

    // 禁用拷贝（纹理资源不应被多处持有）
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    // 支持移动语义，允许纹理对象转移所有权
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    // ==================== 纹理创建与加载 ====================

    /**
     * 从图片文件加载纹理
     * 支持 BMP、PNG、JPG、WEBP 等常见图片格式
     * @param renderer 渲染器对象引用
     * @param filePath 图片文件路径（相对或绝对路径均可）
     * @return 加载成功返回 true，失败返回 false
     */
    bool loadFromFile(Renderer& renderer, const std::string& filePath);

    /**
     * 从内存数据加载纹理
     * 适用于从自定义资源包或网络数据中加载图片
     * @param renderer 渲染器对象引用
     * @param data     图片数据的内存指针
     * @param dataSize 图片数据的大小（字节）
     * @return 加载成功返回 true，失败返回 false
     */
    bool loadFromMemory(Renderer& renderer, const void* data, size_t dataSize);

    /**
     * 创建一个空白的纹理（用于渲染目标或程序化生成）
     * @param renderer   渲染器对象引用
     * @param width      纹理宽度
     * @param height     纹理高度
     * @param access     访问模式：
     *                   - SDL_TEXTUREACCESS_STATIC:   内容很少变化，不能锁定
     *                   - SDL_TEXTUREACCESS_STREAMING: 内容频繁变化，可以锁定像素
     *                   - SDL_TEXTUREACCESS_TARGET:    可以作为渲染目标
     * @return 创建成功返回 true，失败返回 false
     */
    bool createBlank(Renderer& renderer, int width, int height,
                     SDL_TextureAccess access = SDL_TEXTUREACCESS_TARGET);

    /**
     * 从 SDL_Surface 创建纹理
     * 适用于程序化生成图像后转换为纹理的场景
     * @param renderer 渲染器对象引用
     * @param surface  SDL_Surface 指针，创建后此表面可由调用者自行释放
     * @return 创建成功返回 true，失败返回 false
     */
    bool createFromSurface(Renderer& renderer, SDL_Surface* surface);

    /**
     * 销毁纹理，释放资源
     * 销毁后可以重新加载新纹理
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
     * 设置纹理的颜色调制（整体着色）
     * 将纹理每个像素的颜色与指定颜色相乘
     * 例如：白色纹理 (255,255,255) 设置 (255,0,0) 后变为红色
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
     * @param blendMode 混合模式：
     *   - SDL_BLENDMODE_NONE:    无混合，直接覆盖
     *   - SDL_BLENDMODE_BLEND:   标准Alpha混合（最常用）
     *   - SDL_BLENDMODE_ADD:     加法混合（适合光效、火焰）
     *   - SDL_BLENDMODE_MOD:     乘法混合（适合阴影、暗化）
     *   - SDL_BLENDMODE_MUL:     颜色乘法混合
     * @return 成功返回 true
     */
    bool setBlendMode(SDL_BlendMode blendMode);

private:
    SDL_Texture* mTexture;  // SDL 原生纹理指针
    int mWidth;             // 纹理宽度缓存
    int mHeight;            // 纹理高度缓存
};

#endif // TEXTURE_H
