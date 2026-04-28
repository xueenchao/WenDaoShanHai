#include "UIProgressBar.h"
#include "../Renderer.h"
#include "../Font.h"
#include <algorithm>
#include <cstdio>

UIProgressBar::UIProgressBar()
    : mValue(50.0f), mMaxValue(100.0f)
    , mBgR(40), mBgG(40), mBgB(40)
    , mFgR(200), mFgG(50), mFgB(50)
    , mLabelR(255), mLabelG(255), mLabelB(255)
{
}

void UIProgressBar::onRender(Renderer& renderer, Font* font)
{
    float wx, wy;
    getWorldPosition(wx, wy);

    // 背景
    renderer.setDrawColor(mBgR, mBgG, mBgB, 220);
    renderer.fillRect(wx, wy, mWidth, mHeight);

    // 前景（按比例）
    float ratio = (mMaxValue > 0.0f)
        ? std::max(0.0f, std::min(mValue / mMaxValue, 1.0f))
        : 0.0f;
    if (ratio > 0.0f) {
        renderer.setDrawColor(mFgR, mFgG, mFgB, 255);
        renderer.fillRect(wx, wy, mWidth * ratio, mHeight);
    }

    // 文字标签（居中覆盖在进度条上）
    if (font != nullptr && !mLabel.empty()) {
        SDL_Texture* tex = font->renderText(renderer, mLabel, mLabelR, mLabelG, mLabelB);
        if (tex != nullptr) {
            float tw, th;
            SDL_GetTextureSize(tex, &tw, &th);
            float tx = wx + (mWidth - tw) * 0.5f;
            float ty = wy + (mHeight - th) * 0.5f;
            renderer.drawTexture(tex, tx, ty);
            SDL_DestroyTexture(tex);
        }
    }

    // 子元素
    for (auto& child : mChildren) {
        if (child->mVisible) {
            child->onRender(renderer, font);
        }
    }
}
