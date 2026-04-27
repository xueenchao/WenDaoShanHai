/**
 * Renderer.cpp - 渲染器管理类的实现
 *
 * 封装 SDL3 2D 渲染 API，提供简洁的绘制接口。
 */

#include "Renderer.h"
#include "Window.h"

// ==================== 构造与析构 ====================

Renderer::Renderer(Window& window, const std::string& driverName)
    : mWindow(window)          // 保存窗口引用
    , mRenderer(nullptr)       // 渲染器尚未创建
    , mDriverName(driverName)  // 缓存驱动名称
{
}

Renderer::~Renderer()
{
    // 析构时自动销毁渲染器
    destroy();
}

// ==================== 渲染器生命周期 ====================

bool Renderer::create()
{
    // 如果渲染器已存在，先销毁旧的
    if (mRenderer != nullptr) {
        destroy();
    }

    // 获取窗口的原生指针
    SDL_Window* rawWindow = mWindow.getRawWindow();
    if (rawWindow == nullptr) {
        SDL_Log("Renderer::create 失败: 关联的窗口尚未创建");
        return false;
    }

    // 调用 SDL3 API 创建渲染器
    // driverName 为空时，SDL 会自动选择最佳的渲染驱动
    const char* driver = mDriverName.empty() ? nullptr : mDriverName.c_str();
    mRenderer = SDL_CreateRenderer(rawWindow, driver);

    if (mRenderer == nullptr) {
        SDL_Log("Renderer::create 失败: %s", SDL_GetError());
        return false;
    }

    return true;
}

void Renderer::destroy()
{
    if (mRenderer != nullptr) {
        SDL_DestroyRenderer(mRenderer);
        mRenderer = nullptr;
    }
}

bool Renderer::isValid() const
{
    return mRenderer != nullptr;
}

// ==================== 渲染基础操作 ====================

SDL_Renderer* Renderer::getRawRenderer() const
{
    return mRenderer;
}

bool Renderer::clear()
{
    if (mRenderer == nullptr) {
        return false;
    }
    // SDL_RenderClear 用当前绘图颜色清空整个渲染目标
    return SDL_RenderClear(mRenderer);
}

void Renderer::present()
{
    if (mRenderer == nullptr) {
        return;
    }
    // SDL_RenderPresent 将后台缓冲区内容呈现到屏幕
    SDL_RenderPresent(mRenderer);
}

// ==================== 绘图颜色 ====================

void Renderer::setDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    if (mRenderer == nullptr) {
        return;
    }
    SDL_SetRenderDrawColor(mRenderer, r, g, b, a);
}

void Renderer::getDrawColor(Uint8& r, Uint8& g, Uint8& b, Uint8& a) const
{
    if (mRenderer == nullptr) {
        r = g = b = a = 0;
        return;
    }
    SDL_GetRenderDrawColor(mRenderer, &r, &g, &b, &a);
}

// ==================== 逻辑分辨率 ====================

bool Renderer::setLogicalSize(int width, int height)
{
    if (mRenderer == nullptr) {
        return false;
    }
    // SDL_LOGICAL_PRESENTATION_LETTERBOX 保持宽高比，不足部分留黑边
    // 这是游戏开发中最常用的模式，确保画面不会拉伸变形
    return SDL_SetRenderLogicalPresentation(
        mRenderer, width, height, SDL_LOGICAL_PRESENTATION_LETTERBOX);
}

void Renderer::getLogicalSize(int& width, int& height) const
{
    if (mRenderer == nullptr) {
        width = 0;
        height = 0;
        return;
    }
    SDL_RendererLogicalPresentation mode;
    SDL_GetRenderLogicalPresentation(mRenderer, &width, &height, &mode);
}

// ==================== 绘制基本图元 ====================

bool Renderer::drawLine(float x1, float y1, float x2, float y2)
{
    if (mRenderer == nullptr) {
        return false;
    }
    return SDL_RenderLine(mRenderer, x1, y1, x2, y2);
}

bool Renderer::drawRect(float x, float y, float width, float height)
{
    if (mRenderer == nullptr) {
        return false;
    }
    // SDL3 使用 SDL_FRect 结构来描述浮点矩形
    SDL_FRect rect = { x, y, width, height };
    return SDL_RenderRect(mRenderer, &rect);
}

bool Renderer::fillRect(float x, float y, float width, float height)
{
    if (mRenderer == nullptr) {
        return false;
    }
    SDL_FRect rect = { x, y, width, height };
    return SDL_RenderFillRect(mRenderer, &rect);
}

bool Renderer::drawPoint(float x, float y)
{
    if (mRenderer == nullptr) {
        return false;
    }
    // SDL3 中画点使用 SDL_RenderLine 画一个长度为0的线
    // 或者直接用 SDL_RenderRect 画一个 1x1 的矩形
    SDL_FRect point = { x, y, 1.0f, 1.0f };
    return SDL_RenderFillRect(mRenderer, &point);
}

// ==================== 纹理渲染 ====================

bool Renderer::drawTexture(SDL_Texture* texture, float dstX, float dstY)
{
    if (mRenderer == nullptr || texture == nullptr) {
        return false;
    }
    // 获取纹理的原始大小，用于确定目标区域
    float texWidth = 0.0f;
    float texHeight = 0.0f;
    SDL_GetTextureSize(texture, &texWidth, &texHeight);

    // 整个纹理绘制到 (dstX, dstY) 位置，保持原始大小
    SDL_FRect dstRect = { dstX, dstY, texWidth, texHeight };
    // srcrect 传 nullptr 表示使用整个纹理
    return SDL_RenderTexture(mRenderer, texture, nullptr, &dstRect);
}

bool Renderer::drawTexture(SDL_Texture* texture, float dstX, float dstY,
                           float dstWidth, float dstHeight)
{
    if (mRenderer == nullptr || texture == nullptr) {
        return false;
    }
    // 整个纹理缩放绘制到指定的目标区域
    SDL_FRect dstRect = { dstX, dstY, dstWidth, dstHeight };
    return SDL_RenderTexture(mRenderer, texture, nullptr, &dstRect);
}

bool Renderer::drawTexture(SDL_Texture* texture,
                           float srcX, float srcY, float srcWidth, float srcHeight,
                           float dstX, float dstY, float dstWidth, float dstHeight)
{
    if (mRenderer == nullptr || texture == nullptr) {
        return false;
    }
    // 指定源区域（裁剪）和目标区域（缩放）
    SDL_FRect srcRect = { srcX, srcY, srcWidth, srcHeight };
    SDL_FRect dstRect = { dstX, dstY, dstWidth, dstHeight };
    return SDL_RenderTexture(mRenderer, texture, &srcRect, &dstRect);
}

bool Renderer::drawTextureRotated(SDL_Texture* texture,
                                  float dstX, float dstY, float dstWidth, float dstHeight,
                                  double angle, float centerX, float centerY,
                                  SDL_FlipMode flip)
{
    if (mRenderer == nullptr || texture == nullptr) {
        return false;
    }
    SDL_FRect dstRect = { dstX, dstY, dstWidth, dstHeight };
    SDL_FPoint center = { centerX, centerY };
    return SDL_RenderTextureRotated(mRenderer, texture, nullptr, &dstRect,
                                    angle, &center, flip);
}

// ==================== 渲染目标 ====================

bool Renderer::setRenderTarget(SDL_Texture* texture)
{
    if (mRenderer == nullptr) {
        return false;
    }
    return SDL_SetRenderTarget(mRenderer, texture);
}

SDL_Texture* Renderer::getRenderTarget() const
{
    if (mRenderer == nullptr) {
        return nullptr;
    }
    return SDL_GetRenderTarget(mRenderer);
}

// ==================== 视口 ====================

bool Renderer::setViewport(int x, int y, int width, int height)
{
    if (mRenderer == nullptr) {
        return false;
    }
    SDL_Rect viewport = { x, y, width, height };
    return SDL_SetRenderViewport(mRenderer, &viewport);
}

bool Renderer::resetViewport()
{
    if (mRenderer == nullptr) {
        return false;
    }
    // 传入 nullptr 将视口重置为整个渲染目标
    return SDL_SetRenderViewport(mRenderer, nullptr);
}
