/**
 * Window.cpp - 窗口管理类的实现
 *
 * 对 SDL3 窗口相关 API 进行封装，提供简洁的 C++ 接口。
 */

#include "Window.h"
#include "Log.h"

// ==================== 构造与析构 ====================

Window::Window(const std::string& title, int width, int height, SDL_WindowFlags flags)
    : mWindow(nullptr)       // 初始化为空指针，窗口尚未创建
    , mTitle(title)          // 缓存窗口标题
    , mWidth(width)          // 记录窗口宽度
    , mHeight(height)        // 记录窗口高度
    , mFlags(flags)          // 记录窗口标志
{
    // 构造函数只保存参数，不创建窗口
    // 需要显式调用 create() 来创建窗口
}

Window::~Window()
{
    // 析构时自动销毁窗口，防止资源泄漏
    destroy();
}

// ==================== 窗口生命周期 ====================

bool Window::create()
{
    // 如果窗口已经存在，先销毁旧的，避免内存泄漏
    if (mWindow != nullptr) {
        destroy();
    }

    // 调用 SDL3 API 创建窗口
    // SDL_CreateWindow 参数：标题、宽度、高度、标志
    mWindow = SDL_CreateWindow(mTitle.c_str(), mWidth, mHeight, mFlags);

    if (mWindow == nullptr) {
        // 创建失败，输出错误信息到 SDL 日志
        LOG_ERROR("create 失败: %s", SDL_GetError());
        return false;
    }

    return true;
}

void Window::destroy()
{
    if (mWindow != nullptr) {
        // 调用 SDL3 API 销毁窗口
        SDL_DestroyWindow(mWindow);
        mWindow = nullptr;
    }
}

bool Window::isValid() const
{
    // 窗口指针非空即为有效
    return mWindow != nullptr;
}

// ==================== 窗口属性 ====================

SDL_Window* Window::getRawWindow() const
{
    return mWindow;
}

std::string Window::getTitle() const
{
    // 如果窗口已创建，从 SDL 获取当前标题（可能被外部修改过）
    if (mWindow != nullptr) {
        const char* title = SDL_GetWindowTitle(mWindow);
        if (title != nullptr) {
            return std::string(title);
        }
    }
    // 窗口未创建时返回缓存标题
    return mTitle;
}

void Window::setTitle(const std::string& title)
{
    // 更新缓存
    mTitle = title;

    // 如果窗口已创建，同步更新 SDL 窗口标题
    if (mWindow != nullptr) {
        SDL_SetWindowTitle(mWindow, mTitle.c_str());
    }
}

void Window::getSize(int& width, int& height) const
{
    if (mWindow != nullptr) {
        // 从 SDL 获取窗口的实际大小
        SDL_GetWindowSize(mWindow, &width, &height);
    } else {
        // 窗口未创建时返回记录的初始大小
        width = mWidth;
        height = mHeight;
    }
}

void Window::setSize(int width, int height)
{
    // 更新缓存
    mWidth = width;
    mHeight = height;

    // 如果窗口已创建，同步更新 SDL 窗口大小
    if (mWindow != nullptr) {
        SDL_SetWindowSize(mWindow, mWidth, mHeight);
    }
}

void Window::getPosition(int& x, int& y) const
{
    if (mWindow != nullptr) {
        SDL_GetWindowPosition(mWindow, &x, &y);
    } else {
        x = 0;
        y = 0;
    }
}

void Window::setPosition(int x, int y)
{
    if (mWindow != nullptr) {
        SDL_SetWindowPosition(mWindow, x, y);
    }
}

// ==================== 全屏与显示 ====================

void Window::setFullscreen(bool fullscreen)
{
    if (mWindow == nullptr) {
        return;
    }

    if (fullscreen) {
        // 设置为全屏模式
        // SDL_WINDOW_FULLSCREEN 表示真正的全屏（独占模式）
        // SDL_WINDOW_FULLSCREEN_DESKTOP 表示桌面全屏（无边框窗口，保持桌面分辨率）
        SDL_SetWindowFullscreen(mWindow, true);
    } else {
        // 恢复为窗口模式
        SDL_SetWindowFullscreen(mWindow, false);
    }
}

bool Window::isFullscreen() const
{
    if (mWindow == nullptr) {
        return false;
    }

    // SDL_GetWindowFlags 返回窗口的当前标志位
    SDL_WindowFlags flags = SDL_GetWindowFlags(mWindow);
    // 检查是否包含全屏标志
    return (flags & SDL_WINDOW_FULLSCREEN) != 0;
}

void Window::toggleFullscreen()
{
    // 取反当前全屏状态
    setFullscreen(!isFullscreen());
}
