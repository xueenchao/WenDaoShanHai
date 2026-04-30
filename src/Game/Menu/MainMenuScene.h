/**
 * MainMenuScene.h - 开始菜单场景
 *
 * 简单的标题画面，包含"开始游戏"和"退出游戏"按钮。
 * 使用直接渲染+手动命中检测，避免引入UIManager。
 */

#ifndef MAINMENUSCENE_H
#define MAINMENUSCENE_H

#include "Core/Scene.h"

class EventHandler;
class Renderer;
class Font;

class MainMenuScene : public Scene {
public:
    MainMenuScene(EventHandler& eh, Font* font, float vpW, float vpH);

    void onEnter() override;
    void onExit() override;
    void onUpdate(float deltaTime) override;
    void onRender(Renderer& renderer) override;

    bool shouldStart() const { return mShouldStart; }
    bool shouldQuit() const { return mShouldQuit; }

private:
    EventHandler& mEH;
    Font* mFont;
    float mVpW, mVpH;

    bool mShouldStart = false;
    bool mShouldQuit = false;

    // 按钮区域（屏幕坐标）
    struct Button {
        float x, y, w, h;
        const char* text;
        bool hovered = false;
    };
    Button mStartBtn;
    Button mQuitBtn;

    void updateButton(Button& btn);
};

#endif // MAINMENUSCENE_H
