/**
 * main.cpp - 《问道·山海》
 *
 * 游戏入口：主菜单 → 大世界探索 → 回合制战棋战斗。
 */

#include <SDL3/SDL.h>
#include <cstdlib>
#include <ctime>

#include "Core/Core.h"
#include "Game/PlayerData.h"
#include "Game/DataManager.h"
#include "Game/MainMenuScene.h"
#include "Game/BigWorldScene.h"

// 全局对象
static Window gWindow("问道·山海", 1280, 720);
static Renderer gRenderer(gWindow);
static EventHandler gEventHandler;
static GameLoop gGameLoop(gEventHandler, 60);
static SceneManager gSceneManager(gEventHandler);
static Font* gFont = nullptr;

// 玩家持久数据
static PlayerData gPlayerData;

// ==================== GameLoop 回调 ====================

bool onInit()
{
    srand(static_cast<unsigned int>(time(nullptr)));

    Log::getInstance().setLogToFile(true, "game.log");

    // 加载所有游戏数据
    if (!DataManager::getInstance().loadAll()) {
        LOG_ERROR("游戏数据加载失败！");
        return false;
    }

    const auto& cfg = DataManager::getInstance().getConfig();

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        LOG_ERROR("SDL_Init 失败: %s", SDL_GetError());
        return false;
    }

    if (!gWindow.create()) return false;
    if (!gRenderer.create()) return false;
    gRenderer.setLogicalSize(static_cast<int>(cfg.viewportW), static_cast<int>(cfg.viewportH));

    if (!TTF_Init()) {
        LOG_ERROR("TTF_Init 失败: %s", SDL_GetError());
        return false;
    }

    gFont = new Font();
    if (!cfg.fontPrimary.empty() && gFont->load(cfg.fontPrimary.c_str(), cfg.fontSize)) {
        // 加载成功
    } else if (!cfg.fontFallback.empty() && gFont->load(cfg.fontFallback.c_str(), cfg.fontSize)) {
        // 加载成功
    } else {
        if (!gFont->load("C:/Windows/Fonts/msyh.ttc", 14.0f)) {
            gFont->load("C:/Windows/Fonts/simsun.ttc", 14.0f);
        }
    }

    // 初始化玩家数据（从DataManager加载起始技能和物品）
    gPlayerData = PlayerData();
    for (auto& skillId : DataManager::getInstance().getStartingSkills()) {
        const Skill* sk = DataManager::getInstance().getSkill(skillId);
        if (sk) gPlayerData.skills.push_back(*sk);
    }
    gPlayerData.inventory = DataManager::getInstance().createStartingInventory();

    // 进入主菜单
    gSceneManager.pushScene(std::make_unique<MainMenuScene>(
        gEventHandler, gFont, cfg.viewportW, cfg.viewportH));

    LOG_INFO("===== 问道·山海 =====");

    return true;
}

void onUpdate(float deltaTime)
{
    gSceneManager.onUpdate(deltaTime);

    // 检查是否请求退出
    if (gSceneManager.isQuitRequested()
        || gEventHandler.isKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
        gGameLoop.stop();
        return;
    }

    // 主菜单 → 大世界转换
    Scene* active = gSceneManager.getActiveScene();
    if (active != nullptr && active->mName == "主菜单") {
        auto* menu = static_cast<MainMenuScene*>(active);
        bool start = menu->shouldStart();
        bool quit = menu->shouldQuit();
        if (start) {
            const auto& cfg = DataManager::getInstance().getConfig();
            gSceneManager.switchScene(std::make_unique<BigWorldScene>(
                gEventHandler, gFont, cfg.viewportW, cfg.viewportH,
                &gPlayerData, &gSceneManager));
        }
        if (quit) {
            gGameLoop.stop();
        }
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

    LOG_INFO("===== 游戏结束 =====");
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
