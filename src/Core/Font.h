/**
 * Font.h - 字体与文字渲染类
 *
 * 封装 SDL3_ttf 的字体加载与文字渲染功能。
 * 支持 TrueType 字体（.ttf、.otf）的加载、大小设置和文字绘制。
 *
 * 文字渲染的三种质量模式：
 *   - Solid：  最快，但文字边缘有锯齿，适合大量文字
 *   - Shaded： 带背景色的抗锯齿，文字清晰，但有背景色块
 *   - Blended： 透明抗锯齿，效果最好，速度最慢，适合少量标题文字
 */

#ifndef FONT_H
#define FONT_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>

// 前向声明
class Renderer;

class Font {
public:
    // ==================== 文字渲染质量 ====================

    /**
     * 文字渲染质量枚举
     * 渲染质量越高，效果越好，但速度越慢
     */
    enum class RenderQuality {
        Solid,    // 快速渲染，无抗锯齿，适合调试信息或大量文字
        Shaded,   // 带背景色抗锯齿，文字清晰但有底色块
        Blended   // 透明抗锯齿，效果最佳，适合标题和重要文字
    };

    // ==================== 构造与析构 ====================

    /**
     * 默认构造函数：创建空的字体对象
     * 需要后续调用 load() 来加载字体文件
     */
    Font();

    /**
     * 构造函数：创建并加载字体
     * @param filePath 字体文件路径（.ttf 或 .otf）
     * @param fontSize 字体大小（磅值），常用值：16(小)、24(中)、32(大)、48(标题)
     */
    Font(const std::string& filePath, float fontSize);

    /**
     * 析构函数：自动释放字体资源
     */
    ~Font();

    // 禁用拷贝
    Font(const Font&) = delete;
    Font& operator=(const Font&) = delete;

    // 支持移动语义
    Font(Font&& other) noexcept;
    Font& operator=(Font&& other) noexcept;

    // ==================== 字体加载与管理 ====================

    /**
     * 加载字体文件
     * @param filePath 字体文件路径
     * @param fontSize 字体大小（磅值）
     * @return 加载成功返回 true，失败返回 false
     */
    bool load(const std::string& filePath, float fontSize);

    /**
     * 从内存数据加载字体
     * 适用于从自定义资源包中加载字体
     * @param data     字体数据的内存指针
     * @param dataSize 数据大小（字节）
     * @param fontSize 字体大小
     * @return 加载成功返回 true，失败返回 false
     */
    bool loadFromMemory(const void* data, size_t dataSize, float fontSize);

    /**
     * 销毁字体，释放资源
     */
    void destroy();

    /**
     * 判断字体是否有效
     * @return 有效返回 true
     */
    bool isValid() const;

    // ==================== 字体属性 ====================

    /**
     * 获取 SDL 原生字体指针
     * @return TTF_Font 指针
     */
    TTF_Font* getRawFont() const;

    /**
     * 动态修改字体大小（无需重新加载字体文件）
     * @param fontSize 新的字体大小
     * @return 成功返回 true
     */
    bool setFontSize(float fontSize);

    /**
     * 获取当前字体大小
     * @return 字体大小（磅值）
     */
    float getFontSize() const;

    /**
     * 设置字体样式
     * @param style 样式标志，可组合使用：
     *   - TTF_STYLE_NORMAL:      正常
     *   - TTF_STYLE_BOLD:        粗体
     *   - TTF_STYLE_ITALIC:      斜体
     *   - TTF_STYLE_UNDERLINE:   下划线
     *   - TTF_STYLE_STRIKETHROUGH: 删除线
     *   例如：TTF_STYLE_BOLD | TTF_STYLE_ITALIC 表示粗斜体
     */
    void setStyle(TTF_FontStyleFlags style);

    /**
     * 获取当前字体样式
     * @return 样式标志
     */
    TTF_FontStyleFlags getStyle() const;

    // ==================== 文字渲染 ====================

    /**
     * 将文字渲染为纹理（最常用的方法）
     * 自动将文字渲染为 Surface 再转换为 Texture
     * @param renderer 渲染器对象引用
     * @param text     要渲染的文字（UTF-8编码，支持中文）
     * @param r        文字颜色 - 红色分量 (0-255)
     * @param g        文字颜色 - 绿色分量 (0-255)
     * @param b        文字颜色 - 蓝色分量 (0-255)
     * @param quality  渲染质量，默认 Blended（最佳效果）
     * @return 渲染成功返回 SDL_Texture 指针，调用者负责用 SDL_DestroyTexture 释放；
     *         失败返回 nullptr
     */
    SDL_Texture* renderText(Renderer& renderer, const std::string& text,
                            Uint8 r, Uint8 g, Uint8 b,
                            RenderQuality quality = RenderQuality::Blended);

    /**
     * 将文字渲染为纹理（带背景色，仅 Shaded 模式使用）
     * @param renderer  渲染器对象引用
     * @param text      要渲染的文字
     * @param fgR/fgG/fgB 前景色（文字颜色）
     * @param bgR/bgG/bgB 背景色
     * @return 渲染成功返回 SDL_Texture 指针，失败返回 nullptr
     */
    SDL_Texture* renderTextShaded(Renderer& renderer, const std::string& text,
                                  Uint8 fgR, Uint8 fgG, Uint8 fgB,
                                  Uint8 bgR, Uint8 bgG, Uint8 bgB);

    /**
     * 将多行文字渲染为纹理（自动换行）
     * @param renderer   渲染器对象引用
     * @param text       要渲染的文字
     * @param r/g/b      文字颜色
     * @param wrapWidth  最大行宽（像素），超过此宽度自动换行
     * @param quality    渲染质量
     * @return 渲染成功返回 SDL_Texture 指针，失败返回 nullptr
     */
    SDL_Texture* renderTextWrapped(Renderer& renderer, const std::string& text,
                                   Uint8 r, Uint8 g, Uint8 b,
                                   int wrapWidth,
                                   RenderQuality quality = RenderQuality::Blended);

    /**
     * 测量文字的渲染尺寸（不实际渲染）
     * 用于布局计算，例如居中对齐、换行判断等
     * @param text   要测量的文字
     * @param width  输出：文字宽度（像素）
     * @param height 输出：文字高度（像素）
     * @return 成功返回 true
     */
    bool measureText(const std::string& text, int& width, int& height);

private:
    TTF_Font* mFont;      // SDL_ttf 原生字体指针
    float mFontSize;      // 当前字体大小缓存
};

#endif // FONT_H
