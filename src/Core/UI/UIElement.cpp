#include "UIElement.h"

UIElement::UIElement()
    : mX(0.0f), mY(0.0f)
    , mWidth(0.0f), mHeight(0.0f)
    , mParent(nullptr)
    , mVisible(true)
    , mEnabled(true)
{
}

void UIElement::getWorldPosition(float& wx, float& wy) const
{
    wx = mX;
    wy = mY;
    const UIElement* p = mParent;
    while (p != nullptr) {
        wx += p->mX;
        wy += p->mY;
        p = p->mParent;
    }
}

bool UIElement::containsPoint(float px, float py) const
{
    float wx, wy;
    getWorldPosition(wx, wy);
    return px >= wx && px <= wx + mWidth
        && py >= wy && py <= wy + mHeight;
}

void UIElement::addChild(std::unique_ptr<UIElement> child)
{
    child->mParent = this;
    mChildren.push_back(std::move(child));
}

UIElement* UIElement::hitTest(float px, float py)
{
    if (!mVisible) return nullptr;

    // 从后往前遍历（后添加的在上面）
    for (auto it = mChildren.rbegin(); it != mChildren.rend(); ++it) {
        UIElement* hit = (*it)->hitTest(px, py);
        if (hit != nullptr) return hit;
    }

    if (containsPoint(px, py) && mEnabled) {
        return this;
    }
    return nullptr;
}
