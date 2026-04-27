/**
 * Window.h - 窗口管理类
 *
 * 封装 SDL3 的窗口创建、销毁与基本属性操作。
 * 提供简洁的接口来管理游戏窗口的标题、大小、全屏等属性。
 */

#ifndef WINDOW_H
#define WINDOW_H

#include <SDL3/SDL.h>
#include <string>

class Window {
public:
    // ==================== 构造与析构 ====================

    /**
     * 构造函数：初始化窗口参数，但不会真正创建窗口
     * @param title    窗口标题
     * @param width    窗口宽度（像素）
     * @param height   窗口高度（像素）
     * @param flags    窗口创建标志（如 SDL_WINDOW_FULLSCREEN 等），默认 0 表示普通窗口
     */
    Window(const std::string& title, int width, int height, SDL_WindowFlags flags = (SDL_WindowFlags)0);

    /**
     * 析构函数：自动销毁窗口并释放资源
     */
    ~Window();

    // 禁用拷贝构造和拷贝赋值，避免窗口指针被多处持有导致双重释放
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    // ==================== 窗口生命周期 ====================

    /**
     * 创建窗口：调用 SDL_CreateWindow 真正创建窗口
     * @return 创建成功返回 true，失败返回 false
     */
    bool create();

    /**
     * 销毁窗口：释放窗口资源
     * 销毁后可以重新调用 create() 再次创建
     */
    void destroy();

    /**
     * 判断窗口是否已创建且有效
     * @return 窗口有效返回 true，否则返回 false
     */
    bool isValid() const;

    // ==================== 窗口属性 ====================

    /**
     * 获取 SDL 原生窗口指针
     * 用于需要直接操作 SDL 窗口的场景（如创建渲染器）
     * @return SDL_Window 指针，如果窗口未创建则返回 nullptr
     */
    SDL_Window* getRawWindow() const;

    /**
     * 获取窗口标题
     * @return 当前窗口标题字符串
     */
    std::string getTitle() const;

    /**
     * 设置窗口标题
     * @param title 新的窗口标题
     */
    void setTitle(const std::string& title);

    /**
     * 获取窗口大小
     * @param width  输出参数，接收窗口宽度
     * @param height 输出参数，接收窗口高度
     */
    void getSize(int& width, int& height) const;

    /**
     * 设置窗口大小
     * @param width  新的窗口宽度
     * @param height 新的窗口高度
     */
    void setSize(int width, int height);

    /**
     * 获取窗口在屏幕上的位置
     * @param x 输出参数，接收窗口左上角 X 坐标
     * @param y 输出参数，接收窗口左上角 Y 坐标
     */
    void getPosition(int& x, int& y) const;

    /**
     * 设置窗口在屏幕上的位置
     * @param x 新的左上角 X 坐标
     * @param y 新的左上角 Y 坐标
     */
    void setPosition(int x, int y);

    // ==================== 全屏与显示 ====================

    /**
     * 设置窗口全屏模式
     * @param fullscreen true 表示全屏，false 表示窗口模式
     */
    void setFullscreen(bool fullscreen);

    /**
     * 判断窗口当前是否为全屏模式
     * @return 全屏返回 true，窗口模式返回 false
     */
    bool isFullscreen() const;

    /**
     * 切换全屏/窗口模式（快捷方法）
     */
    void toggleFullscreen();

private:
    SDL_Window* mWindow;      // SDL 原生窗口指针
    std::string mTitle;       // 窗口标题缓存
    int mWidth;               // 窗口宽度
    int mHeight;              // 窗口高度
    SDL_WindowFlags mFlags;   // 窗口创建标志
};

#endif // WINDOW_H
