#include "UIButton.h"
#include "../Renderer.h"
#include "../Font.h"

UIButton::UIButton()
    : mNormalR(220), mNormalG(220), mNormalB(220)
    , mHoverR(255), mHoverG(255), mHoverB(200)
    , mPressR(255), mPressG(200), mPressB(100)
    , mNormalBgR(50), mNormalBgG(50), mNormalBgB(60)
    , mHoverBgR(70), mHoverBgG(70), mHoverBgB(80)
    , mPressBgR(40), mPressBgG(40), mPressBgB(50)
    , mState(State::Normal)
    , mPressedOnThis(false)
{
}

void UIButton::onRender(Renderer& renderer, Font* font)
{
    float wx, wy;
    getWorldPosition(wx, wy);

    // 选择当前状态的颜色
    Uint8 bgR = mNormalBgR, bgG = mNormalBgG, bgB = mNormalBgB;
    Uint8 textR = mNormalR, textG = mNormalG, textB = mNormalB;

    switch (mState) {
    case State::Hover:
        bgR = mHoverBgR; bgG = mHoverBgG; bgB = mHoverBgB;
        textR = mHoverR; textG = mHoverG; textB = mHoverB;
        break;
    case State::Pressed:
        bgR = mPressBgR; bgG = mPressBgG; bgB = mPressBgB;
        textR = mPressR; textG = mPressG; textB = mPressB;
        break;
    default:
        break;
    }

    // 背景
    renderer.setDrawColor(bgR, bgG, bgB, 220);
    renderer.fillRect(wx, wy, mWidth, mHeight);

    // 边框
    renderer.setDrawColor(textR, textG, textB, 100);
    renderer.drawRect(wx, wy, mWidth, mHeight);

    // 文字居中
    if (font != nullptr && !mText.empty()) {
        SDL_Texture* tex = font->renderText(renderer, mText, textR, textG, textB);
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

void UIButton::onMouseEnter()
{
    mState = State::Hover;
}

void UIButton::onMouseLeave()
{
    mState = State::Normal;
    mPressedOnThis = false;
}

void UIButton::onMouseDown(float px, float py)
{
    (void)px; (void)py;
    mState = State::Pressed;
    mPressedOnThis = true;
}

void UIButton::onMouseUp(float px, float py)
{
    (void)px; (void)py;
    if (mPressedOnThis && containsPoint(px, py)) {
        if (mOnClick) {
            mOnClick();
        }
    }
    mPressedOnThis = false;
    mState = State::Normal;
}
