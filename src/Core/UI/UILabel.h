/**
 * UILabel.h - 文字标签
 *
 * 使用Font渲染单行文字。
 */

#ifndef UILABEL_H
#define UILABEL_H

#include "UIElement.h"
#include <string>

class UILabel : public UIElement {
public:
    UILabel();

    void onRender(Renderer& renderer, Font* font) override;

    std::string mText;
    Uint8 mTextR, mTextG, mTextB;
};

#endif // UILABEL_H
