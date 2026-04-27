/**
 * Renderer.h - 渲染器管理类
 *
 * 封装 SDL3 的 2D 渲染器，提供绘制基本图元、纹理渲染、
 * 逻辑分辨率设置等功能。所有绘制操作都需要通过渲染器完成。
 */

#ifndef RENDERER_H
#define RENDERER_H

#include <SDL3/SDL.h>
#include <string>

// 前向声明，避免头文件循环依赖
class Window;

class Renderer {
public:
    // ==================== 构造与析构 ====================

    /**
     * 构造函数：初始化渲染器参数
     * @param window  关联的窗口对象引用
     * @param driverName 渲染驱动名称，传空字符串让 SDL 自动选择最佳驱动
     *                   常见值："" (自动), "opengl", "direct3d", "vulkan", "software"
     */
    Renderer(Window& window, const std::string& driverName = "");

    /**
     * 析构函数：自动销毁渲染器
     */
    ~Renderer();

    // 禁用拷贝，因为渲染器与窗口是唯一绑定的
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    // ==================== 渲染器生命周期 ====================

    /**
     * 创建渲染器
     * @return 创建成功返回 true，失败返回 false
     */
    bool create();

    /**
     * 销毁渲染器
     */
    void destroy();

    /**
     * 判断渲染器是否有效
     * @return 有效返回 true
     */
    bool isValid() const;

    // ==================== 渲染基础操作 ====================

    /**
     * 获取 SDL 原生渲染器指针
     * @return SDL_Renderer 指针，未创建时返回 nullptr
     */
    SDL_Renderer* getRawRenderer() const;

    /**
     * 清空画布（用当前绘图颜色填充整个画面）
     * 通常在每帧开始时调用
     * @return 成功返回 true
     */
    bool clear();

    /**
     * 将后台缓冲区的内容呈现到屏幕上
     * 每帧绘制完成后必须调用，否则画面不会更新
     */
    void present();

    // ==================== 绘图颜色 ====================

    /**
     * 设置绘图颜色（影响 clear、画线、画矩形等操作）
     * @param r 红色分量 (0-255)
     * @param g 绿色分量 (0-255)
     * @param b 蓝色分量 (0-255)
     * @param a 透明度分量 (0-255)，255 表示完全不透明
     */
    void setDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);

    /**
     * 获取当前绘图颜色
     * @param r 输出：红色分量
     * @param g 输出：绿色分量
     * @param b 输出：蓝色分量
     * @param a 输出：透明度分量
     */
    void getDrawColor(Uint8& r, Uint8& g, Uint8& b, Uint8& a) const;

    // ==================== 逻辑分辨率 ====================

    /**
     * 设置逻辑分辨率（虚拟分辨率）
     * 设置后，所有绘制坐标都会基于此虚拟分辨率进行缩放
     * 例如：设置逻辑分辨率为 1280x720，在 1920x1080 的窗口中
     *       绘制 (640, 360) 会自动映射到窗口中心
     * @param width  逻辑宽度
     * @param height 逻辑高度
     * @return 成功返回 true
     */
    bool setLogicalSize(int width, int height);

    /**
     * 获取当前逻辑分辨率
     * @param width  输出：逻辑宽度
     * @param height 输出：逻辑高度
     */
    void getLogicalSize(int& width, int& height) const;

    // ==================== 绘制基本图元 ====================

    /**
     * 画一条直线
     * @param x1 起点X坐标
     * @param y1 起点Y坐标
     * @param x2 终点X坐标
     * @param y2 终点Y坐标
     * @return 成功返回 true
     */
    bool drawLine(float x1, float y1, float x2, float y2);

    /**
     * 画一个矩形边框
     * @param x      矩形左上角X坐标
     * @param y      矩形左上角Y坐标
     * @param width  矩形宽度
     * @param height 矩形高度
     * @return 成功返回 true
     */
    bool drawRect(float x, float y, float width, float height);

    /**
     * 画一个填充矩形
     * @param x      矩形左上角X坐标
     * @param y      矩形左上角Y坐标
     * @param width  矩形宽度
     * @param height 矩形高度
     * @return 成功返回 true
     */
    bool fillRect(float x, float y, float width, float height);

    /**
     * 画一个点（单个像素）
     * @param x 点的X坐标
     * @param y 点的Y坐标
     * @return 成功返回 true
     */
    bool drawPoint(float x, float y);

    // ==================== 纹理渲染 ====================

    /**
     * 将纹理整体绘制到渲染目标上
     * @param texture 要绘制的纹理指针
     * @param dstX    目标位置左上角X坐标
     * @param dstY    目标位置左上角Y坐标
     * @return 成功返回 true
     */
    bool drawTexture(SDL_Texture* texture, float dstX, float dstY);

    /**
     * 将纹理绘制到指定区域（可缩放）
     * @param texture   要绘制的纹理指针
     * @param dstX      目标区域左上角X坐标
     * @param dstY      目标区域左上角Y坐标
     * @param dstWidth  目标区域宽度
     * @param dstHeight 目标区域高度
     * @return 成功返回 true
     */
    bool drawTexture(SDL_Texture* texture, float dstX, float dstY, float dstWidth, float dstHeight);

    /**
     * 将纹理的指定区域绘制到目标的指定区域（裁剪+缩放）
     * @param texture   要绘制的纹理指针
     * @param srcX      源区域左上角X坐标
     * @param srcY      源区域左上角Y坐标
     * @param srcWidth  源区域宽度
     * @param srcHeight 源区域高度
     * @param dstX      目标区域左上角X坐标
     * @param dstY      目标区域左上角Y坐标
     * @param dstWidth  目标区域宽度
     * @param dstHeight 目标区域高度
     * @return 成功返回 true
     */
    bool drawTexture(SDL_Texture* texture,
                     float srcX, float srcY, float srcWidth, float srcHeight,
                     float dstX, float dstY, float dstWidth, float dstHeight);

    /**
     * 将纹理旋转绘制到指定区域
     * @param texture   要绘制的纹理指针
     * @param dstX      目标区域左上角X坐标
     * @param dstY      目标区域左上角Y坐标
     * @param dstWidth  目标区域宽度
     * @param dstHeight 目标区域高度
     * @param angle     旋转角度（度数，顺时针方向）
     * @param centerX   旋转中心X偏移（相对于目标区域左上角），传 nullptr 则以中心旋转
     * @param centerY   旋转中心Y偏移（同上）
     * @param flip      翻转模式：SDL_FLIP_NONE / SDL_FLIP_HORIZONTAL / SDL_FLIP_VERTICAL
     * @return 成功返回 true
     */
    bool drawTextureRotated(SDL_Texture* texture,
                            float dstX, float dstY, float dstWidth, float dstHeight,
                            double angle, float centerX, float centerY,
                            SDL_FlipMode flip = SDL_FLIP_NONE);

    // ==================== 渲染目标 ====================

    /**
     * 设置渲染目标（默认是屏幕）
     * 传入 nullptr 恢复默认渲染目标（即屏幕）
     * @param texture 渲染目标纹理，必须是用 SDL_TEXTUREACCESS_TARGET 创建的
     * @return 成功返回 true
     */
    bool setRenderTarget(SDL_Texture* texture);

    /**
     * 获取当前渲染目标
     * @return 当前渲染目标纹理，如果是屏幕则返回 nullptr
     */
    SDL_Texture* getRenderTarget() const;

    // ==================== 视口 ====================

    /**
     * 设置渲染视口（绘制区域）
     * 只在视口区域内进行绘制，超出部分被裁剪
     * @param x      视口左上角X坐标
     * @param y      视口左上角Y坐标
     * @param width  视口宽度
     * @param height 视口高度
     * @return 成功返回 true
     */
    bool setViewport(int x, int y, int width, int height);

    /**
     * 重置视口为整个屏幕
     * @return 成功返回 true
     */
    bool resetViewport();

private:
    Window& mWindow;              // 关联的窗口引用
    SDL_Renderer* mRenderer;      // SDL 原生渲染器指针
    std::string mDriverName;      // 渲染驱动名称
};

#endif // RENDERER_H
