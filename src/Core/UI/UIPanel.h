/**
 * UIPanel.h - 面板容器
 *
 * 带背景色的矩形容器，可包含子元素。
 */

#ifndef UIPANEL_H
#define UIPANEL_H

#include "UIElement.h"

class UIPanel : public UIElement {
public:
    UIPanel();

    void onRender(Renderer& renderer, Font* font) override;

    Uint8 mBgR, mBgG, mBgB, mBgA;
};

#endif // UIPANEL_H
