/**
 * main.cpp - 《问道·山海》游戏入口
 *
 * 演示核心层各模块的用法：
 *   1. 初始化 SDL 各子系统
 *   2. 创建窗口与渲染器
 *   3. 设置游戏主循环
 *   4. 在循环中处理输入、更新逻辑、渲染画面
 *   5. 退出时清理资源
 */

// 定义 SDL_MAIN_HANDLED 让我们自己控制主函数
// （已在 CMakeLists.txt 中通过 target_compile_definitions 添加）
#include <SDL3/SDL.h>

#include "Core/Core.h"

// ==================== 全局对象 ====================

// 窗口：标题 "问道·山海"，大小 1280x720，普通窗口模式
static Window gWindow("问道·山海", 1280, 720);

// 渲染器：关联上面的窗口，让 SDL 自动选择渲染驱动
static Renderer gRenderer(gWindow);

// 音频系统：44100Hz 采样率，立体声
static Audio gAudio(44100, 2);

// 字体：用于显示中文文字
static Font gFont;

// 事件处理器：管理键盘/鼠标/窗口事件
static EventHandler gEventHandler;

// 游戏主循环：目标 60 FPS
static GameLoop gGameLoop(gEventHandler, 60);

// 示例用的纹理和文字纹理指针
static Texture gTestTexture;
static SDL_Texture* gTitleTextTexture = nullptr;
static int gTitleTextWidth = 0;
static int gTitleTextHeight = 0;

// 简单的移动测试变量
static float gPlayerX = 580.0f;
static float gPlayerY = 300.0f;
static float gPlayerSpeed = 300.0f;   // 每秒移动 300 像素

// ==================== 初始化回调 ====================

bool onInit()
{
    // ----- 步骤1：初始化 SDL 核心系统 -----
    // SDL_INIT_VIDEO  包含了 VIDEO 和 EVENTS 子系统
    // SDL_INIT_AUDIO  音频子系统
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_Log("SDL_Init 失败: %s", SDL_GetError());
        return false;
    }

    // ----- 步骤2：创建窗口 -----
    if (!gWindow.create()) {
        SDL_Log("窗口创建失败");
        return false;
    }

    // ----- 步骤3：创建渲染器 -----
    if (!gRenderer.create()) {
        SDL_Log("渲染器创建失败");
        return false;
    }

    // 设置逻辑分辨率为 1280x720
    // 这样无论窗口实际大小如何，绘制坐标都基于 1280x720
    gRenderer.setLogicalSize(1280, 720);

    // ----- 步骤4：初始化音频系统 -----
    if (!gAudio.init()) {
        SDL_Log("音频系统初始化失败（不影响游戏运行）");
        // 音频初始化失败不阻止游戏运行，只是没有声音
    }

    // ----- 步骤5：初始化 SDL_ttf 字体系统 -----
    if (!TTF_Init()) {
        SDL_Log("TTF_Init 失败: %s", SDL_GetError());
        return false;
    }

    // ----- 步骤6：加载字体 -----
    // 注意：这里需要你提供一个真实的字体文件路径
    // 例如 Windows 系统自带的微软雅黑：
    //   "C:/Windows/Fonts/msyh.ttc"
    // 或者项目中自带的 .ttf 文件
    // 如果字体加载失败，文字将不会显示，但不影响其他功能
    if (!gFont.load("C:/Windows/Fonts/msyh.ttc", 36.0f)) {
        SDL_Log("字体加载失败，文字将不会显示");
    }

    // ----- 步骤7：渲染标题文字纹理 -----
    if (gFont.isValid()) {
        // 先测量文字尺寸
        gFont.measureText("问道·山海", gTitleTextWidth, gTitleTextHeight);

        // 渲染文字为纹理（白金色文字）
        gTitleTextTexture = gFont.renderText(gRenderer, "问道·山海", 255, 215, 0);
    }

    // ----- 步骤8：加载测试图片 -----
    // 注意：这里需要你提供一个真实的图片文件路径
    // 如果图片不存在，程序仍可运行，只是看不到图片
    // gTestTexture.loadFromFile(gRenderer, "assets/test.png");

    SDL_Log("游戏初始化完成！按 ESC 键退出");

    return true;
}

// ==================== 更新回调 ====================

void onUpdate(float deltaTime)
{
    // ----- 键盘移动测试 -----
    // 使用 WASD 或方向键移动一个方块
    float moveAmount = gPlayerSpeed * deltaTime;

    if (gEventHandler.isKeyPressed(SDL_SCANCODE_A) ||
        gEventHandler.isKeyPressed(SDL_SCANCODE_LEFT)) {
        gPlayerX -= moveAmount;
    }
    if (gEventHandler.isKeyPressed(SDL_SCANCODE_D) ||
        gEventHandler.isKeyPressed(SDL_SCANCODE_RIGHT)) {
        gPlayerX += moveAmount;
    }
    if (gEventHandler.isKeyPressed(SDL_SCANCODE_W) ||
        gEventHandler.isKeyPressed(SDL_SCANCODE_UP)) {
        gPlayerY -= moveAmount;
    }
    if (gEventHandler.isKeyPressed(SDL_SCANCODE_S) ||
        gEventHandler.isKeyPressed(SDL_SCANCODE_DOWN)) {
        gPlayerY += moveAmount;
    }

    // 限制移动范围在逻辑分辨率内
    if (gPlayerX < 0.0f) gPlayerX = 0.0f;
    if (gPlayerX > 1280.0f - 32.0f) gPlayerX = 1280.0f - 32.0f;
    if (gPlayerY < 0.0f) gPlayerY = 0.0f;
    if (gPlayerY > 720.0f - 32.0f) gPlayerY = 720.0f - 32.0f;

    // ----- 按 ESC 退出 -----
    if (gEventHandler.isKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
        gGameLoop.stop();
    }

    // ----- 按 F11 切换全屏 -----
    if (gEventHandler.isKeyJustPressed(SDL_SCANCODE_F11)) {
        gWindow.toggleFullscreen();
    }
}

// ==================== 渲染回调 ====================

void onRender()
{
    // ----- 清空画布（深蓝色背景，营造修仙氛围） -----
    gRenderer.setDrawColor(10, 15, 40, 255);
    gRenderer.clear();

    // ----- 绘制网格线（展示逻辑分辨率和绘图功能） -----
    gRenderer.setDrawColor(30, 40, 70, 255);
    for (int x = 0; x < 1280; x += 64) {
        gRenderer.drawLine(static_cast<float>(x), 0.0f, static_cast<float>(x), 720.0f);
    }
    for (int y = 0; y < 720; y += 64) {
        gRenderer.drawLine(0.0f, static_cast<float>(y), 1280.0f, static_cast<float>(y));
    }

    // ----- 绘制玩家方块（金色，可键盘移动） -----
    gRenderer.setDrawColor(255, 200, 50, 255);
    gRenderer.fillRect(gPlayerX, gPlayerY, 32.0f, 32.0f);

    // 给方块加个边框
    gRenderer.setDrawColor(255, 255, 150, 255);
    gRenderer.drawRect(gPlayerX, gPlayerY, 32.0f, 32.0f);

    // ----- 绘制标题文字 -----
    if (gTitleTextTexture != nullptr) {
        // 计算居中位置
        float titleX = (1280.0f - static_cast<float>(gTitleTextWidth)) / 2.0f;
        float titleY = 60.0f;
        gRenderer.drawTexture(gTitleTextTexture, titleX, titleY);
    }

    // ----- 绘制提示文字 -----
    if (gFont.isValid()) {
        SDL_Texture* hintTexture = gFont.renderText(
            gRenderer, "WASD移动 | F11全屏 | ESC退出", 150, 160, 180);
        if (hintTexture != nullptr) {
            gRenderer.drawTexture(hintTexture, 440.0f, 660.0f);
            SDL_DestroyTexture(hintTexture);
        }
    }

    // ----- 绘制帧率信息 -----
    if (gFont.isValid()) {
        // 将帧率转为字符串
        float fps = gGameLoop.getCurrentFPS();
        std::string fpsText = "FPS: " + std::to_string(static_cast<int>(fps));
        SDL_Texture* fpsTexture = gFont.renderText(
            gRenderer, fpsText, 100, 200, 100);
        if (fpsTexture != nullptr) {
            gRenderer.drawTexture(fpsTexture, 10.0f, 10.0f);
            SDL_DestroyTexture(fpsTexture);
        }
    }

    // ----- 呈现画面 -----
    gRenderer.present();
}

// ==================== 清理回调 ====================

void onCleanup()
{
    // 释放文字纹理
    if (gTitleTextTexture != nullptr) {
        SDL_DestroyTexture(gTitleTextTexture);
        gTitleTextTexture = nullptr;
    }

    // 销毁测试纹理
    gTestTexture.destroy();

    // 销毁字体
    gFont.destroy();

    // 关闭 SDL_ttf
    TTF_Quit();

    // 关闭音频系统
    gAudio.shutdown();

    // 销毁渲染器（必须先于窗口销毁）
    gRenderer.destroy();

    // 销毁窗口
    gWindow.destroy();

    // 关闭 SDL
    SDL_Quit();

    SDL_Log("游戏已退出，下次再会！");
}

// ==================== 主函数 ====================

int main(int argc, char* argv[])
{
    // 忽略命令行参数
    (void)argc;
    (void)argv;

    // 设置游戏循环的回调函数
    gGameLoop.setOnInit(onInit);
    gGameLoop.setOnUpdate(onUpdate);
    gGameLoop.setOnRender(onRender);
    gGameLoop.setOnCleanup(onCleanup);

    // 启动游戏主循环
    // run() 会阻塞直到游戏退出
    gGameLoop.run();

    return 0;
}
