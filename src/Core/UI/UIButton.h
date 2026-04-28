/**
 * UIButton.h - 按钮控件
 *
 * 带文字的可点击按钮，支持悬停/按下/正常三态颜色。
 */

#ifndef UIBUTTON_H
#define UIBUTTON_H

#include "UIElement.h"
#include <functional>
#include <string>

class UIButton : public UIElement {
public:
    UIButton();

    void onRender(Renderer& renderer, Font* font) override;
    void onMouseEnter() override;
    void onMouseLeave() override;
    void onMouseDown(float px, float py) override;
    void onMouseUp(float px, float py) override;

    std::string mText;

    // 三态颜色
    Uint8 mNormalR, mNormalG, mNormalB;
    Uint8 mHoverR, mHoverG, mHoverB;
    Uint8 mPressR, mPressG, mPressB;

    // 对应状态的背景色
    Uint8 mNormalBgR, mNormalBgG, mNormalBgB;
    Uint8 mHoverBgR, mHoverBgG, mHoverBgB;
    Uint8 mPressBgR, mPressBgG, mPressBgB;

    std::function<void()> mOnClick;

private:
    enum class State { Normal, Hover, Pressed };
    State mState;

    bool mPressedOnThis;
};

#endif // UIBUTTON_H
