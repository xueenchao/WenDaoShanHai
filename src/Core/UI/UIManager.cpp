#include "UIManager.h"

UIManager::UIManager(Font* font)
    : mRenderer(nullptr)
    , mFont(font)
    , mRoot(std::make_unique<UIElement>())
    , mHoveredElement(nullptr)
    , mPressedElement(nullptr)
{
}

void UIManager::render()
{
    if (mRenderer == nullptr) return;

    for (auto& child : mRoot->mChildren) {
        if (child->mVisible) {
            child->onRender(*mRenderer, mFont);
        }
    }
}

UIElement* UIManager::getElementAt(float mx, float my)
{
    return mRoot->hitTest(mx, my);
}

void UIManager::updateHover(float mx, float my)
{
    UIElement* target = getElementAt(mx, my);
    if (target != mHoveredElement) {
        if (mHoveredElement != nullptr) {
            mHoveredElement->onMouseLeave();
        }
        mHoveredElement = target;
        if (mHoveredElement != nullptr) {
            mHoveredElement->onMouseEnter();
        }
    }
}

void UIManager::handleMouseDown(float mx, float my)
{
    mPressedElement = getElementAt(mx, my);
    if (mPressedElement != nullptr) {
        mPressedElement->onMouseDown(mx, my);
    }
}

void UIManager::handleMouseUp(float mx, float my)
{
    if (mPressedElement != nullptr) {
        mPressedElement->onMouseUp(mx, my);
    }
    mPressedElement = nullptr;
}

void UIManager::addElement(std::unique_ptr<UIElement> element)
{
    mRoot->addChild(std::move(element));
}

void UIManager::clear()
{
    mRoot->mChildren.clear();
    mHoveredElement = nullptr;
    mPressedElement = nullptr;
}
