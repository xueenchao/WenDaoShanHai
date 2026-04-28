/**
 * main.cpp - 《问道·山海》战斗系统测试
 *
 * 测试回合制战棋战斗：2个玩家单位 vs 3个敌方单位
 * 在8x6网格上进行移动、技能释放和回合交替。
 */

#include <SDL3/SDL.h>
#include <cstdlib>
#include <ctime>

#include "Core/Core.h"
#include "Game/CombatScene.h"

constexpr float VIEWPORT_W = 1280.0f;
constexpr float VIEWPORT_H = 720.0f;

// 全局对象
static Window gWindow("问道·山海 - 战斗测试", 1280, 720);
static Renderer gRenderer(gWindow);
static EventHandler gEventHandler;
static GameLoop gGameLoop(gEventHandler, 60);
static SceneManager gSceneManager(gEventHandler);
static Font* gFont = nullptr;

// ==================== GameLoop 回调 ====================

bool onInit()
{
    srand(static_cast<unsigned int>(time(nullptr)));

    // 初始化日志文件
    Log::getInstance().setLogToFile(true, "game.log");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        LOG_ERROR("SDL_Init 失败: %s", SDL_GetError());
        return false;
    }

    if (!gWindow.create()) return false;
    if (!gRenderer.create()) return false;
    gRenderer.setLogicalSize(static_cast<int>(VIEWPORT_W), static_cast<int>(VIEWPORT_H));

    if (!TTF_Init()) {
        LOG_ERROR("TTF_Init 失败: %s", SDL_GetError());
        return false;
    }

    // 加载字体
    gFont = new Font();
    if (!gFont->load("C:/Windows/Fonts/msyh.ttc", 14.0f)) {
        gFont->load("C:/Windows/Fonts/simsun.ttc", 14.0f);
    }

    // 进入战斗场景
    gSceneManager.pushScene(std::make_unique<CombatScene>(
        gEventHandler, gFont, VIEWPORT_W, VIEWPORT_H));

    LOG_INFO("===== 问道·山海 战斗系统测试 =====");
    LOG_INFO("操作: 左键点击蓝色格子移动 | 右键取消技能");
    LOG_INFO("      点击右侧技能按钮 -> 点击红色标记敌人 -> 释放技能");
    LOG_INFO("      点击底部\"结束回合\"按钮结束当前回合");
    LOG_INFO("      ESC 退出");

    return true;
}

void onUpdate(float deltaTime)
{
    gSceneManager.onUpdate(deltaTime);

    if (gSceneManager.isQuitRequested()
        || gEventHandler.isKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
        gGameLoop.stop();
    }
}

void onRender()
{
    gSceneManager.onRender(gRenderer);
    gRenderer.present();
}

void onCleanup()
{
    gSceneManager.onCleanup();

    delete gFont;
    gFont = nullptr;

    if (TTF_WasInit()) TTF_Quit();
    gRenderer.destroy();
    gWindow.destroy();
    SDL_Quit();

    LOG_INFO("===== 战斗系统测试结束 =====");
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    gGameLoop.setOnInit(onInit);
    gGameLoop.setOnUpdate(onUpdate);
    gGameLoop.setOnRender(onRender);
    gGameLoop.setOnCleanup(onCleanup);

    gGameLoop.run();
    return 0;
}
