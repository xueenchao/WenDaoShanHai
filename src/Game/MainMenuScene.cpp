/**
 * MainMenuScene.cpp - 开始菜单场景实现
 */

#include "MainMenuScene.h"
#include "Core/EventHandler.h"
#include "Core/Renderer.h"
#include "Core/Font.h"
#include "Core/Log.h"
#include <SDL3/SDL.h>

MainMenuScene::MainMenuScene(EventHandler& eh, Font* font, float vpW, float vpH)
    : Scene("主菜单")
    , mEH(eh)
    , mFont(font)
    , mVpW(vpW)
    , mVpH(vpH)
{
}

void MainMenuScene::onEnter()
{
    LOG_INFO("进入主菜单");

    mShouldStart = false;
    mShouldQuit = false;

    // 开始按钮
    mStartBtn.x = mVpW * 0.5f - 100.0f;
    mStartBtn.y = mVpH * 0.55f;
    mStartBtn.w = 200.0f;
    mStartBtn.h = 50.0f;
    mStartBtn.text = "开始游戏";

    // 退出按钮
    mQuitBtn.x = mVpW * 0.5f - 100.0f;
    mQuitBtn.y = mVpH * 0.55f + 70.0f;
    mQuitBtn.w = 200.0f;
    mQuitBtn.h = 50.0f;
    mQuitBtn.text = "退出游戏";
}

void MainMenuScene::onExit()
{
    LOG_INFO("离开主菜单");
}

void MainMenuScene::updateButton(Button& btn)
{
    float mx, my;
    mEH.getMousePosition(mx, my);

    btn.hovered = (mx >= btn.x && mx <= btn.x + btn.w
                && my >= btn.y && my <= btn.y + btn.h);
}

void MainMenuScene::onUpdate(float /*deltaTime*/)
{
    updateButton(mStartBtn);
    updateButton(mQuitBtn);

    if (mEH.isMouseButtonJustPressed(SDL_BUTTON_LEFT)) {
        if (mStartBtn.hovered) {
            mShouldStart = true;
            LOG_INFO("点击开始游戏");
        }
        if (mQuitBtn.hovered) {
            mShouldQuit = true;
            LOG_INFO("点击退出游戏");
        }
    }
}

void MainMenuScene::onRender(Renderer& renderer)
{
    // 背景
    renderer.setDrawColor(10, 12, 18, 255);
    renderer.clear();

    // 标题
    if (mFont) {
        SDL_Texture* titleTex = mFont->renderText(renderer, "问道·山海",
            255, 215, 0, Font::RenderQuality::Blended);
        if (titleTex) {
            float tw, th;
            SDL_GetTextureSize(titleTex, &tw, &th);
            renderer.drawTexture(titleTex,
                mVpW * 0.5f - tw * 0.5f,
                mVpH * 0.25f - th * 0.5f);
            SDL_DestroyTexture(titleTex);
        }

        SDL_Texture* subTex = mFont->renderText(renderer, "—— 仙侠世界 · 山海经 ——",
            160, 180, 200, Font::RenderQuality::Solid);
        if (subTex) {
            float tw, th;
            SDL_GetTextureSize(subTex, &tw, &th);
            renderer.drawTexture(subTex,
                mVpW * 0.5f - tw * 0.5f,
                mVpH * 0.25f + 50.0f);
            SDL_DestroyTexture(subTex);
        }
    }

    // 绘制按钮
    auto drawBtn = [&](const Button& btn) {
        if (btn.hovered) {
            renderer.setDrawColor(60, 50, 30, 255);
        } else {
            renderer.setDrawColor(40, 35, 25, 255);
        }
        renderer.fillRect(btn.x, btn.y, btn.w, btn.h);

        renderer.setDrawColor(180, 150, 80, 255);
        renderer.drawRect(btn.x, btn.y, btn.w, btn.h);

        if (mFont) {
            SDL_Texture* tex = mFont->renderText(renderer, btn.text,
                220, 200, 160, Font::RenderQuality::Solid);
            if (tex) {
                float tw, th;
                SDL_GetTextureSize(tex, &tw, &th);
                renderer.drawTexture(tex,
                    btn.x + (btn.w - tw) * 0.5f,
                    btn.y + (btn.h - th) * 0.5f);
                SDL_DestroyTexture(tex);
            }
        }
    };

    drawBtn(mStartBtn);
    drawBtn(mQuitBtn);

    // 底部版本信息
    if (mFont) {
        SDL_Texture* verTex = mFont->renderText(renderer, "v0.1 — 道基阶段",
            100, 100, 100, Font::RenderQuality::Solid);
        if (verTex) {
            float tw, th;
            SDL_GetTextureSize(verTex, &tw, &th);
            renderer.drawTexture(verTex,
                mVpW * 0.5f - tw * 0.5f,
                mVpH - 40.0f);
            SDL_DestroyTexture(verTex);
        }
    }
}
