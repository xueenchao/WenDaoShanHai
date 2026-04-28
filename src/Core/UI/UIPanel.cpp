#include "UIPanel.h"
#include "../Renderer.h"

UIPanel::UIPanel()
    : mBgR(30), mBgG(30), mBgB(40), mBgA(220)
{
}

void UIPanel::onRender(Renderer& renderer, Font* font)
{
    // 渲染背景
    float wx, wy;
    getWorldPosition(wx, wy);
    renderer.setDrawColor(mBgR, mBgG, mBgB, mBgA);
    renderer.fillRect(wx, wy, mWidth, mHeight);

    // 渲染子元素
    for (auto& child : mChildren) {
        if (child->mVisible) {
            child->onRender(renderer, font);
        }
    }
}
