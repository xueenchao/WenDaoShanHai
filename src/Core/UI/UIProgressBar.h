/**
 * UIProgressBar.h - 进度条控件
 *
 * 双色矩形条，用于显示HP、灵力、经验值等比率。
 */

#ifndef UIPROGRESSBAR_H
#define UIPROGRESSBAR_H

#include "UIElement.h"
#include <string>

class UIProgressBar : public UIElement {
public:
    UIProgressBar();

    void onRender(Renderer& renderer, Font* font) override;

    float mValue;
    float mMaxValue;

    Uint8 mBgR, mBgG, mBgB;      // 背景色
    Uint8 mFgR, mFgG, mFgB;      // 前景色

    std::string mLabel;           // 可选的文字标签（如 "HP 45/100"）
    Uint8 mLabelR, mLabelG, mLabelB;
};

#endif // UIPROGRESSBAR_H
